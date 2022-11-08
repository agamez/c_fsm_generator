#ifndef __fsm_h__
#define __fsm_h__

#include <stdarg.h>
#include <stddef.h>

#include <pthread.h>
#include "freebsd-queue.h"

struct fsm;
typedef void (*free_data_cb)(void *data);
typedef int (*fsm_process_event_cb)(struct fsm *fsm);

#define FSM_ST_INITIAL_STATE 0

enum fsm_states {
	FSM_ST_NULL,
	FSM_ST_NUMBER_OF_STATES
};

struct fsm_state {
	enum fsm_states code;
	const char *name;
	int (*enter)(struct fsm *fsm);
	int (*exit)(struct fsm *fsm);
	const fsm_process_event_cb *process_event_table;

	void *data;
};

struct fsm_event {
	int code;
	const char *name;
	const char *description;

	int is_allocated;

	void *data;
	free_data_cb free_data_cb;
};

enum fsm_event_codes {
	FSM_EV_NULL,
	FSM_EV_NUMBER_OF_EVENTS
};

struct fsm_event_member {
	struct fsm_event *event;
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

	struct fsm_event *last_event;

	int first_state_entered;

	pthread_mutex_t locked_fsm;
};

void fsm_debug(struct fsm *fsm, int priority, const char *format, ...);
int fsm_init(struct fsm *fsm, void *data);
int fsm_enter(struct fsm *fsm);
int fsm_exit(struct fsm *fsm);
int fsm_process_event(struct fsm *fsm, struct fsm_event *event);

void fsm_fifo_add_event(struct fsm *fsm, struct fsm_event *event);
int fsm_fifo_process_events(struct fsm *fsm);


#endif /* __state_h__ */
