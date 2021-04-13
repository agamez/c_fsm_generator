#ifndef __art_fsm_h__
#define __art_fsm_h__

#include "art_fsm_states.h"

struct art_fsm;

struct art_fsm_state {
	enum art_fsm_states code;
	const char *name;
	struct art_fsm_event (*enter)(struct art_fsm *fsm);
	struct art_fsm_event (*exit)(struct art_fsm *fsm);
	const struct art_fsm_state *(*process_event)(struct art_fsm *fsm, struct art_fsm_event *event);
};

struct art_fsm {
	const struct art_fsm_state *state;
	const struct art_fsm_state **states;
};

struct art_fsm_event {
	int code;
	const char *name;
	const char *description;
	void *data;
};

struct art_fsm_event fsm_init(struct art_fsm *fsm);
struct art_fsm_event fsm_enter(struct art_fsm *fsm);
struct art_fsm_event fsm_exit(struct art_fsm *fsm);
void fsm_process_event(struct art_fsm *fsm, struct art_fsm_event *event);

#include "art_fsm_events.h"
#include "art_fsm_transitions.h"

#endif /* __art_state_h__ */
