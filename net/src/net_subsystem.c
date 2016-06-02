
#include "aroop/aroop_core.h"
#include "aroop/core/xtring.h"
#include "nginz_config.h"
#include "log.h"
#include <signal.h>
#include "plugin_manager.h"
#include "fiber.h"
#include "fork.h"
#include "db.h"
#include "shake/quitall.h"
#include "streamio.h"
#include "parallel/pipeline.h"
#include "net_subsystem.h"
#include "raw_pipeline_internal.h"
#include "tcp_listener_internal.h"
#include "protostack.h"

C_CAPSULE_START

int nginz_net_init_after_parallel_init() {
	if(!is_master())
		return 0;
	tcp_listener_init();
	return 0;
}

int nginz_net_init() {
	protostack_init();
	pp_raw_module_init();
	return 0;
}

int nginz_net_deinit() {
	tcp_listener_deinit();
	pp_raw_module_deinit();
	protostack_deinit();
	return 0;
}


C_CAPSULE_END
