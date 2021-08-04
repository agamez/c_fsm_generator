#include <stddef.h>

#include "../art_sar_fsm.h"

static const struct fsm_event *booting_enter(struct fsm *fsm)
{
	// if (perform_initialization() < 0)
	//	return fsm->events[ART_SAR_FSM_EV_ERROR];
	return art_sar_fsm_event(fsm, ART_SAR_FSM_EV_INITIALIZE);
}

static const struct fsm_event *booting_exit(struct fsm *fsm)
{
	return NULL;
}

static const struct fsm_state *booting_process_event(struct fsm *fsm, const struct fsm_event *event)
{
	return NULL;
}

const struct fsm_state art_sar_fsm_state_booting = {
	.name = "BOOTING",
	.code = ART_SAR_FSM_ST_BOOTING,
	.enter = booting_enter,
	.exit = booting_exit,
	.process_event = booting_process_event,
};

