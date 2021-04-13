#include <assert.h>
#include <stdio.h>

#include "fsm.h"

struct art_fsm_event fsm_init(struct art_fsm *fsm)
{
	printf("Initializing FSM\n\n");

	fsm->states = art_fsm_status;
	fsm->state = fsm->states[ART_FSM_ST_INITIAL_STATE];

	return fsm_enter(fsm);
}

struct art_fsm_event fsm_enter(struct art_fsm *fsm)
{
	assert(fsm && fsm->state);
	printf("Entering status '%s'\n", fsm->state->name);

	return fsm->state->enter(fsm);
}

struct art_fsm_event fsm_exit(struct art_fsm *fsm)
{
	assert(fsm && fsm->state);
	printf("Exiting status '%s'\n\n", fsm->state->name);

	return fsm->state->exit(fsm);
}

void fsm_process_event(struct art_fsm *fsm, struct art_fsm_event *event)
{
	assert(fsm && fsm->state);
	assert(event);

	if (event->code == ART_FSM_EV_NULL) {
		printf("Ignoring NULL event\n");
		return;
	}
	printf("Processing event '%s' while on status '%s'\n", event->name, fsm->state->name);

	const struct art_fsm_state *new_state = fsm->state->process_event(fsm, event);

	/* If no new_state is received, standard transition is applied. Otherwise, let the state override matrix */
	new_state = new_state ? new_state : fsm->states[art_fsm_transition[fsm->state->code][event->code]];

	if (new_state != ART_FSM_ST_NULL) {
		fsm_exit(fsm);
		fsm->state = new_state;
		fsm_enter(fsm);
	}
}
