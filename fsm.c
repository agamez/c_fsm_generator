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

int fsm_enter(struct fsm *fsm)
{
	assert(fsm && fsm->state);
	fsm_debug(fsm, LOG_NOTICE, "ENTER\n");

	int ret = 0;
	if (fsm->state->enter)
		ret = fsm->state->enter(fsm);

	uint64_t inc = 1;
	write(fsm->state_changed_fd, &inc, sizeof(inc));

	return ret;
}

int fsm_exit(struct fsm *fsm)
{
	assert(fsm && fsm->state);
	fsm_debug(fsm, LOG_NOTICE, "EXIT\n");

	if (fsm->state->exit)
		return fsm->state->exit(fsm);

	return 0;
}

void fsm_fifo_add_event(struct fsm *fsm, struct fsm_event *event)
{
	struct fsm_event_member *m = calloc(1, sizeof(*m));
	m->event = event;

	pthread_mutex_lock(&fsm->fifo_mutex);
	STAILQ_INSERT_TAIL(&fsm->fifo, m, fifo);

	uint64_t inc = 1;
	write(fsm->fifo_added_fd, &inc, sizeof(inc));
	pthread_mutex_unlock(&fsm->fifo_mutex);
}

int fsm_fifo_process_events(struct fsm *fsm)
{
	int ret = 0;
	uint64_t dec;

	do {
		/* First, try to gather exclusive access to the FIFO */
		pthread_mutex_lock(&fsm->fifo_mutex);

		ret = read(fsm->fifo_added_fd, &dec, sizeof(dec));
		/* If we couldn't read from fifo_added_fd, it means that some thread got here before us */
		if (ret < 0) {
			pthread_mutex_unlock(&fsm->fifo_mutex);
			return 0;
		}

		/* If we were able to decrement fifo_added_fd, there must be something in the FIFO that belongs to us */
		struct fsm_event_member *m;
		m = STAILQ_FIRST(&fsm->fifo);
		STAILQ_REMOVE(&fsm->fifo, m, fsm_event_member, fifo);

		pthread_mutex_unlock(&fsm->fifo_mutex);

		/* Once we own the FSM we can proceed with the processing */
		pthread_mutex_lock(&fsm->locked_fsm);

		/* Before processing any event we need to have entered first state */
		if (!fsm->first_state_entered) {
			ret = fsm_enter(fsm);
			fsm->first_state_entered = 1;
		}

		if (m)
			ret = fsm_process_event(fsm, m->event);

		free(m);

		pthread_mutex_unlock(&fsm->locked_fsm);
	} while (dec > 0);


	return ret;
}

int fsm_process_event(struct fsm *fsm, struct fsm_event *event)
{
	int ret = 0;

	assert(fsm && fsm->state);
	if (!event || event->code == FSM_EV_NULL)
		return ret;

	pthread_mutex_lock(&fsm->locked_fsm);

	fsm_debug(fsm, LOG_NOTICE, "EVENT %s\n", event->name);

	fsm->last_event = event;

	if (fsm->state->process_event_table[event->code]) {
		ret = (fsm->state->process_event_table[event->code])(fsm);
		if (ret < 0)
			goto out;
	}

	/* If a new state was returned by the processing function, jump to it.
	 * Otherwise, use the transition matrix to gather the new state */

	const enum fsm_states *transition = (const enum fsm_states *)fsm->transitions;
	transition += fsm->state->code * fsm->n_events + event->code; // Index fsm->transitions[fsm->state->code][event->code]
	const struct fsm_state *new_state = fsm->states[*transition];

	if (new_state) {
		fsm_exit(fsm);
		fsm->prev_state = fsm->state;
		fsm->state = new_state;
		fsm_enter(fsm);
	}

	if (event->is_allocated) {
		if (event->free_data_cb)
			event->free_data_cb(event->data);
		free(event);
		fsm->last_event = NULL;
	}

out:
	pthread_mutex_unlock(&fsm->locked_fsm);

	return 0;
}
