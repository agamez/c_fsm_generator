#ifndef __fsm_h__
#define __fsm_h__

#include <stddef.h>

struct fsm;

#define FSM_ST_INITIAL_STATE 0

enum fsm_states {
	FSM_ST_NULL,
	FSM_ST_NUMBER_OF_STATES
};

struct fsm_state {
	enum fsm_states code;
	const char *name;
	const struct fsm_event *(*enter)(struct fsm *fsm);
	const struct fsm_event *(*exit)(struct fsm *fsm);
	const struct fsm_event *(*process_event)(struct fsm *fsm, const struct fsm_event *event);

	void *data;
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

struct fsm {
	void (*debug)(int priority, const char *format, ...);

	int n_states;
	int n_events;

	const struct fsm_event *events;
	const struct fsm_state *state;
	const struct fsm_state **states;
	const enum fsm_states ***transitions;

	void *data;
};


const struct fsm_event *fsm_init(struct fsm *fsm, void *data);
const struct fsm_event *fsm_enter(struct fsm *fsm);
const struct fsm_event *fsm_exit(struct fsm *fsm);
const struct fsm_event *fsm_process_event(struct fsm *fsm, const struct fsm_event *event);

#endif /* __state_h__ */
