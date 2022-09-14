#define _GNU_SOURCE
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include <syslog.h>
#include <unistd.h>

#include "fsm.h"

const void fsm_debug(struct fsm *fsm, int priority, const char *format, ...)
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

const struct fsm_event *fsm_init(struct fsm *fsm, void *data)
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

	return fsm_enter(fsm);
}

const struct fsm_event *fsm_enter(struct fsm *fsm)
{
	assert(fsm && fsm->state);
	uint64_t inc = 1;
	write(fsm->state_changed_fd, &inc, sizeof(inc));
	fsm_debug(fsm, LOG_NOTICE, "ENTER\n");

	return fsm->state->enter(fsm);
}

const struct fsm_event *fsm_exit(struct fsm *fsm)
{
	assert(fsm && fsm->state);
	fsm_debug(fsm, LOG_NOTICE, "EXIT\n");

	return fsm->state->exit(fsm);
}

const struct fsm_event *fsm_process_event(struct fsm *fsm, const struct fsm_event *event)
{
	assert(fsm && fsm->state);
	if (!event || event->code == FSM_EV_NULL) {
		return NULL;
	}
	fsm_debug(fsm, LOG_NOTICE, "EVENT %s\n", event->name);

	const struct fsm_event *new_event = fsm->state->process_event(fsm, event);
	/* If an event was returned it means we are skipping the transition to new state
	 * and are instead returning this event for further processing.
	 * Typically this is a transition to an error state, but could be any other thing
	 */
	if (new_event)
		return new_event;

	/* By default, however, we search for the new state on transition matrix */
	const enum fsm_states *transition = (const enum fsm_states *)fsm->transitions;
	transition += fsm->state->code * fsm->n_events + event->code; // Index fsm->transitions[fsm->state->code][event->code]
	const struct fsm_state *new_state = fsm->states[*transition];

	/* Transition to new state */
	if (new_state) {
		const struct fsm_event *exit_event, *enter_event;
		exit_event = fsm_exit(fsm);
		fsm->prev_state = fsm->state;
		fsm->state = new_state;
		enter_event = fsm_enter(fsm);

		fsm_process_event(fsm, exit_event);
		fsm_process_event(fsm, enter_event);
	}

	return new_event;
}
