#include "fsm.h"
#include "art_fsm.h"

int main(int argc, char *argv[])
{
	struct art_fsm fsm;

	struct art_fsm_event *fsm_event;

	fsm_event = art_fsm_init(&fsm);
	art_fsm_process_event(&fsm, fsm_event);

	fsm_event = &art_fsm_events[ART_FSM_EV_TRIGGER];
	art_fsm_process_event(&fsm, fsm_event);

	fsm_event = &art_fsm_events[ART_FSM_EV_ERROR];
	art_fsm_process_event(&fsm, fsm_event);

	fsm_event = &art_fsm_events[ART_FSM_EV_RESET];
	art_fsm_process_event(&fsm, fsm_event);

/*
	fsm_process_event(&fsm, &fsm_event);
	fsm_process_event(&fsm, &fsm_event);
*/
}
