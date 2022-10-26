#ifndef __fsm_h__
#define __fsm_h__

#include <stdarg.h>
#include <stddef.h>

#include <pthread.h>
#include "freebsd-queue.h"

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

struct fsm_event_member {
	const struct fsm_event *event;
	STAILQ_ENTRY(fsm_event_member) fifo;
};

struct fsm {
	void (*debug)(struct fsm *fsm, int priority, const char *format, va_list ap);
	/* Can be used with epoll() to gather when state has changed */
	int state_changed_fd;

	const char *name;

	int n_states;
	int n_events;

	const struct fsm_event *events;
	const struct fsm_state *state, *prev_state;
	const struct fsm_state **states;
	const enum fsm_states ***transitions;

	void *data;

	/* Can be used with epoll() to gather when new event is available on the FIFO */
	int fifo_added_fd;
	STAILQ_HEAD(fifo, fsm_event_member) fifo;
	pthread_mutex_t fifo_mutex;
};

void fsm_debug(struct fsm *fsm, int priority, const char *format, ...);
const struct fsm_event *fsm_init(struct fsm *fsm, void *data);
const struct fsm_event *fsm_enter(struct fsm *fsm);
const struct fsm_event *fsm_exit(struct fsm *fsm);
int fsm_process_event(struct fsm *fsm, struct fsm_event *event);

const void fsm_fifo_add_event(struct fsm *fsm, const struct fsm_event *event);
int fsm_fifo_process_event(struct fsm *fsm);


#endif /* __state_h__ */
