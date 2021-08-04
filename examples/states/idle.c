#include <stddef.h>

#include "../art_sar_fsm.h"

static const struct fsm_event *idle_enter(struct fsm *fsm)
{
	return &fsm->events[ART_SAR_FSM_EV_NULL];
}

static const struct fsm_event *idle_exit(struct fsm *fsm)
{
	return NULL;
}

static const struct fsm_state *idle_process_event(struct fsm *fsm, const struct fsm_event *event)
{
	return NULL;
}

const struct fsm_state art_sar_fsm_state_idle = {
	.name = "IDLE",
	.code = ART_SAR_FSM_ST_IDLE,
	.enter = idle_enter,
	.exit = idle_exit,
	.process_event = idle_process_event,
};

