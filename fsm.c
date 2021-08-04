#include <assert.h>
#include <stdio.h>

#include "fsm.h"

const struct fsm_event *fsm_init(struct fsm *fsm, void *data)
{
	assert(fsm);
	if (fsm->debug)
		printf("Initializing FSM\n\n");

	static const struct fsm_state *fsm_states[] = {
		NULL,
		NULL
	};

	static const enum fsm_states fsm_transition[FSM_ST_NUMBER_OF_STATES][FSM_EV_NUMBER_OF_EVENTS] = {
	};


	fsm->states = fsm_states;
	fsm->state = fsm->states[FSM_ST_INITIAL_STATE];
	fsm->transitions = (const enum fsm_states ***)fsm_transition;
	fsm->data = data;

	return fsm_enter(fsm);
}

const struct fsm_event *fsm_enter(struct fsm *fsm)
{
	assert(fsm && fsm->state);
	if (fsm->debug)
		printf("Entering status '%s'\n", fsm->state->name);

	return fsm->state->enter(fsm);
}

const struct fsm_event *fsm_exit(struct fsm *fsm)
{
	assert(fsm && fsm->state);
	if (fsm->debug)
		printf("Exiting status '%s'\n\n", fsm->state->name);

	return fsm->state->exit(fsm);
}

const struct fsm_event *fsm_process_event(struct fsm *fsm, const struct fsm_event *event)
{
	assert(fsm && fsm->state);
	if (!event || event->code == FSM_EV_NULL) {
		if (fsm->debug)
			printf("Ignoring NULL event\n");
		return NULL;
	}
	if (fsm->debug)
		printf("FSM: Processing event '%s' while on status '%s'\n", event->name, fsm->state->name);

	const struct fsm_state *new_state = fsm->state->process_event(fsm, event);

	/* If no new_state is received, standard transition is applied. Otherwise, let the state override matrix */
	const enum fsm_states *transition = (const enum fsm_states *)fsm->transitions;
	transition += fsm->state->code * fsm->n_events + event->code; // Index fsm->transitions[fsm->state->code][event->code]
	new_state = new_state ? new_state : fsm->states[*transition];

	if (new_state) {
		fsm_exit(fsm);
		fsm->state = new_state;
		fsm_enter(fsm);
	}

	return NULL;
}
