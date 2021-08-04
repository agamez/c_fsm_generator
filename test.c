#include "fsm.h"
#include "art_sar_fsm.h"

int main(int argc, char *argv[])
{
	struct fsm fsm = {
		.debug = 1,
	};

	const struct fsm_event *fsm_event;

	fsm_event = art_sar_fsm_init(&fsm, NULL);
	fsm_process_event(&fsm, fsm_event);

	fsm_event = art_sar_fsm_event(&fsm, ART_SAR_FSM_EV_TRIGGER);
	fsm_process_event(&fsm, fsm_event);

	fsm_event = art_sar_fsm_event(&fsm, ART_SAR_FSM_EV_ERROR);
	fsm_process_event(&fsm, fsm_event);

	fsm_event = art_sar_fsm_event(&fsm, ART_SAR_FSM_EV_RESET);
	fsm_process_event(&fsm, fsm_event);

/*
	fsm_process_event(&fsm, &fsm_event);
	fsm_process_event(&fsm, &fsm_event);
*/
}
