#include "fsm.h"

int main(int argc, char *argv[])
{
	struct art_fsm fsm;

	struct art_fsm_event fsm_event;

	fsm_event = fsm_init(&fsm);
	fsm_process_event(&fsm, &fsm_event);

	fsm_event = art_fsm_events[ART_FSM_EV_TRIGGER];
	fsm_process_event(&fsm, &fsm_event);

	fsm_event = art_fsm_events[ART_FSM_EV_ERROR];
	fsm_process_event(&fsm, &fsm_event);

	fsm_event = art_fsm_events[ART_FSM_EV_RESET];
	fsm_process_event(&fsm, &fsm_event);

/*
	fsm_process_event(&fsm, &fsm_event);
	fsm_process_event(&fsm, &fsm_event);
*/
}
