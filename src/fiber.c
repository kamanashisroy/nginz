
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include <aroop/aroop_memory_profiler.h>
#include "plugin_manager.h"
#include "fiber.h"

C_CAPSULE_START

#define MAX_FIBERS 16
struct internal_fiber {
	int status;
	int (*fiber)(int status);
};

static struct internal_fiber fibers[MAX_FIBERS];

int register_fiber(int (*fiber)(int status)) {
	int i = 0;
	for(i = 0; i < MAX_FIBERS; i++) {
		if(fibers[i].status != FIBER_STATUS_EMPTY)
			continue;
		fibers[i].status = FIBER_STATUS_CREATED;
		fibers[i].fiber = fiber;
		return 0;
	}
	aroop_assert(!"Fiber array is full");
	return -1;
}

int unregister_fiber(int (*fiber)(int status)) {
	int i = 0;
	for(i = 0; i < MAX_FIBERS; i++) {
		if(fibers[i].fiber != fiber)
			continue;
		fiber(FIBER_STATUS_DESTROYED);
		fibers[i].status = FIBER_STATUS_EMPTY;
		fibers[i].fiber = NULL;
		return 0;
	}
	return -1;
}

static int fiber_show_count(aroop_txt_t*input, aroop_txt_t*output) {
	int count = 0;
	int i = 0;
	for(i = 0; i < MAX_FIBERS; i++) {
		if(fibers[i].status == FIBER_STATUS_ACTIVATED)
			count++;
	}
	printf("%d fibers\n", count);
	return 0;
}

int fiber_module_init() {
	memset(fibers, 0, sizeof(fibers));
	aroop_txt_t status_plug;
	aroop_txt_embeded_set_static_string(&status_plug, "status");
	pm_plug_callback(&status_plug, fiber_show_count);
}

int fiber_module_deinit() {
	// TODO unregister all
}

int fiber_module_step() {
	int i = 0;
	for(i = 0; i < MAX_FIBERS; i++) {
		if(fibers[i].status == FIBER_STATUS_EMPTY || fibers[i].status == FIBER_STATUS_DEACTIVATED || fibers[i].status == FIBER_STATUS_DESTROYED)
			continue;
		int ret = fibers[i].fiber(fibers[i].status);
		if(ret == -1) {
			unregister_fiber(fibers[i].fiber);
		} else if(ret == -2) {
			fibers[i].status = FIBER_STATUS_DEACTIVATED;
		}
		if(fibers[i].status == FIBER_STATUS_CREATED)
			fibers[i].status = FIBER_STATUS_ACTIVATED;
		return 0;
	}
	return 0;
}

int fiber_module_run() {
	while(1) {
		fiber_module_step();
	}
}

C_CAPSULE_END
