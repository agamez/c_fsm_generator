#ifndef __fsm_h__
#define __fsm_h__

#include <stddef.h>

struct fsm;

#define FSM_ST_INITIAL_STATE 0

enum fsm_states {
	FSM_ST_NULL,
	FSM_ST_NUMBER_OF_STATES
};

static const struct fsm_state *fsm_states[] = {
	NULL,
	NULL
};

struct fsm_state {
	enum fsm_states code;
	const char *name;
	struct fsm_event *(*enter)(struct fsm *fsm);
	struct fsm_event *(*exit)(struct fsm *fsm);
	struct fsm_state *(*process_event)(struct fsm *fsm, struct fsm_event *event);
};

struct fsm_event {
	int code;
	const char *name;
	const char *description;
	void *data;
};

enum fsm_event_codes {
	FSM_EV_NULL,
	FSM_EV_NUMBER_OF_EVENTS
};

static const struct fsm_event fsm_events[] = {
};

struct fsm {
	int n_states;
	int n_events;

	const struct fsm_state *state;
	const struct fsm_state **states;
	const enum fsm_states ***transitions;
};


static enum fsm_states fsm_transition[FSM_ST_NUMBER_OF_STATES][FSM_EV_NUMBER_OF_EVENTS] = {
};

struct fsm_event *fsm_init(struct fsm *fsm);
struct fsm_event *fsm_enter(struct fsm *fsm);
struct fsm_event *fsm_exit(struct fsm *fsm);
struct fsm_event *fsm_process_event(struct fsm *fsm, struct fsm_event *event);

#endif /* __state_h__ */
