#include <stddef.h>

#include "../art_sar_fsm.h"

static const struct fsm_event *capturing_enter(struct fsm *fsm)
{
	return &fsm->events[ART_SAR_FSM_EV_NULL];
}

static const struct fsm_event *capturing_exit(struct fsm *fsm)
{
	return NULL;
}

static const struct fsm_state *capturing_process_event(struct fsm *fsm, const struct fsm_event *event)
{
	return NULL;
}

const struct fsm_state art_sar_fsm_state_capturing = {
	.name = "CAPTURING",
	.code = ART_SAR_FSM_ST_CAPTURING,
	.enter = capturing_enter,
	.exit = capturing_exit,
	.process_event = capturing_process_event,
};

