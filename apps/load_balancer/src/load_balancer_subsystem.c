
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "plugin.h"
#include "log.h"
#include "plugin_manager.h"
#include "parallel/pipeline.h"
#include "load_balancer.h"
#include "load_balancer_subsystem.h"

C_CAPSULE_START

int load_balancer_setup(struct load_balancer*lb) {
	lb->nid = getpid();
	return 0;
}

int load_balancer_next(struct load_balancer*lb) {
	return lb->nid = pp_next_worker_nid(lb->nid);
}

int load_balancer_destroy(struct load_balancer*lb) {
	lb->nid = getpid();
	return 0;
}


int nginz_load_balancer_module_init() {
	return 0;
}

int nginz_load_balancer_module_deinit() {
	return 0;
}


C_CAPSULE_END
