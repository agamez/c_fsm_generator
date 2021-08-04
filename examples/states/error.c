#include <stddef.h>

#include "../art_sar_fsm.h"

static const struct fsm_event *error_enter(struct fsm *fsm)
{
	return &fsm->events[ART_SAR_FSM_EV_NULL];
}

static const struct fsm_event *error_exit(struct fsm *fsm)
{
	return NULL;
}

static const struct fsm_state *error_process_event(struct fsm *fsm, const struct fsm_event *event)
{
	return NULL;
}

const struct fsm_state art_sar_fsm_state_error = {
	.name = "ERROR",
	.code = ART_SAR_FSM_ST_ERROR,
	.enter = error_enter,
	.exit = error_exit,
	.process_event = error_process_event,
};

