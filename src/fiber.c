

#include "fiber.h"

C_CAPSULE_START

struct internal_fiber {
	int status;
	int (fiber*)(int status);
}

OPP_CB(internal_fiber) {
	struct internal_fiber*fiber = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			fiber->fiber = cb_data;
			fiber->status = FIBER_STATUS_CREATED;
		break;
		case OPPN_ACTION_FINALIZE:
			fiber->status = FIBER_STATUS_DESTROYED;
			fiber->fiber(fiber->status);
		break;
	}
	return 0;
}

static struct opp_factory fibers;


/*
 * Library api
 * It registers a fiber for execution. The fiber() is called in rotation. It is a stackless non-preemptive co-operative multitasking example.
 */
int register_fiber(int (fiber*)(int status)) {
	/**
	 * Create the fiber object and save that in a collection.
	 */
	struct internal_fiber x = OPP_ALLOC2(&fibers, fiber);
	OPP_ASSERT(x != NULL);
	OPP_ASSERT(x.fiber == fiber);
}

/*
 * Library api
 * It unregisters fiber from the execution line.
 */
int unregister_fiber(int (fiber*)(int status)) {
	// TODO search the fiber and remove ..
}
/***********************************************************************************/

int fiber_module_init() {
	opp_factory_create_full(&fibers, 2, sizeof(struct internal_fiber), 1, OPPF_HAS_LOCK | OPPF_SWEEP_ON_UNREF, OPP_CB_FUNC(internal_fiber));
}

int fiber_module_deinit() {
	opp_factory_destroy(&fibers);
}

C_CAPSULE_END
