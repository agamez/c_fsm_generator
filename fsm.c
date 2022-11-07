#define _GNU_SOURCE
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include <syslog.h>
#include <unistd.h>

#include "fsm.h"

void fsm_debug(struct fsm *fsm, int priority, const char *format, ...)
{
	if (!fsm->debug)
		return;

	/* Prepend [FSM Name] to debug message */
	char *expanded_format = NULL;
	int ret = asprintf(&expanded_format, "[FSM %s (%d)] %s %s", fsm->name, priority, fsm->state->name, format);

	va_list args;
	va_start(args, format);
	if (ret > 0)
		fsm->debug(fsm, priority, expanded_format, args);

	free(expanded_format);

	va_end(args);	
}

int fsm_init(struct fsm *fsm, void *data)
{
	assert(fsm);
	fsm_debug(fsm, LOG_NOTICE, "Initializing\n");

	static const struct fsm_state *fsm_states[] = {
		NULL,
		NULL
	};

	static const enum fsm_states fsm_transition[FSM_ST_NUMBER_OF_STATES][FSM_EV_NUMBER_OF_EVENTS] = {
	};


	fsm->states = fsm_states;
	fsm->prev_state = NULL;
	fsm->state = fsm->states[FSM_ST_INITIAL_STATE];
	fsm->transitions = (const enum fsm_states ***)fsm_transition;
	fsm->data = data;

	fsm->state_changed_fd = eventfd(0, EFD_CLOEXEC);

	STAILQ_INIT(&fsm->fifo);
	fsm->fifo_added_fd = eventfd(0, EFD_CLOEXEC | EFD_SEMAPHORE | EFD_NONBLOCK);
	pthread_mutex_init(&fsm->fifo_mutex, NULL);

	return fsm_enter(fsm);
}

int fsm_enter(struct fsm *fsm)
{
	assert(fsm && fsm->state);
	uint64_t inc = 1;
	write(fsm->state_changed_fd, &inc, sizeof(inc));
	fsm_debug(fsm, LOG_NOTICE, "ENTER\n");

	return fsm->state->enter(fsm);
}

int fsm_exit(struct fsm *fsm)
{
	assert(fsm && fsm->state);
	fsm_debug(fsm, LOG_NOTICE, "EXIT\n");

	return fsm->state->exit(fsm);
}

void fsm_fifo_add_event(struct fsm *fsm, struct fsm_event *event)
{
	if (!event)
		return;

	struct fsm_event_member *m = calloc(1, sizeof(*m));
	m->event = event;

	pthread_mutex_lock(&fsm->fifo_mutex);

	STAILQ_INSERT_TAIL(&fsm->fifo, m, fifo);

	uint64_t inc = 1;
	write(fsm->fifo_added_fd, &inc, sizeof(inc));

	pthread_mutex_unlock(&fsm->fifo_mutex);
}

int fsm_fifo_process_event(struct fsm *fsm)
{
	pthread_mutex_lock(&fsm->fifo_mutex);

	struct fsm_event_member *m = STAILQ_FIRST(&fsm->fifo);

	uint64_t dec;
	read(fsm->fifo_added_fd, &dec, sizeof(dec));

	pthread_mutex_unlock(&fsm->fifo_mutex);

	int ret = fsm_process_event(fsm, m->event);
	STAILQ_REMOVE_HEAD(&fsm->fifo, fifo);
	free(m);

	return ret;
}

void fsm_change_state(struct fsm *fsm, const struct fsm_state *new_state)
{
	if (new_state) {
		fsm_exit(fsm);
		fsm->prev_state = fsm->state;
		fsm->state = new_state;
		fsm_enter(fsm);
	}
}

int fsm_process_event(struct fsm *fsm, struct fsm_event *event)
{
	assert(fsm && fsm->state);
	if (!event || event->code == FSM_EV_NULL)
		return -1;

	fsm_debug(fsm, LOG_NOTICE, "EVENT %s\n", event->name);

	fsm->last_event = event;

	if (fsm->state->process_event_table[event->code]) {
		int ret = (fsm->state->process_event_table[event->code])(fsm);
		if (ret < 0)
			return -1;
	}

	/* If a new state was returned by the processing function, jump to it.
	 * Otherwise, use the transition matrix to gather the new state */

	const enum fsm_states *transition = (const enum fsm_states *)fsm->transitions;
	transition += fsm->state->code * fsm->n_events + event->code; // Index fsm->transitions[fsm->state->code][event->code]
	const struct fsm_state *new_state = fsm->states[*transition];

	fsm_change_state(fsm, new_state);

	if (event->is_allocated) {
		if (event->free_data_cb)
			event->free_data_cb(event->data);
		free(event);
		fsm->last_event = NULL;
	}

	return 0;
}
