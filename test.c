#include "fsm.h"
#include "art_sar_fsm.h"

int main(int argc, char *argv[])
{
	struct art_sar_fsm fsm;

	const struct art_sar_fsm_event *fsm_event;

	fsm_event = art_sar_fsm_init(&fsm);
	art_sar_fsm_process_event(&fsm, fsm_event);

	fsm_event = &art_sar_fsm_events[ART_SAR_FSM_EV_TRIGGER];
	art_sar_fsm_process_event(&fsm, fsm_event);

	fsm_event = &art_sar_fsm_events[ART_SAR_FSM_EV_ERROR];
	art_sar_fsm_process_event(&fsm, fsm_event);

	fsm_event = &art_sar_fsm_events[ART_SAR_FSM_EV_RESET];
	art_sar_fsm_process_event(&fsm, fsm_event);

/*
	fsm_process_event(&fsm, &fsm_event);
	fsm_process_event(&fsm, &fsm_event);
*/
}
