
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
#include "event_loop.h"
#include "net_subsystem.h"

C_CAPSULE_START

int nginz_net_init_after_parallel_init() {
	if(!is_master())
		return 0;
	tcp_listener_init();
	return 0;
}

int nginz_net_init() {
	protostack_init();
	return 0;
}

int nginz_net_deinit() {
	tcp_listener_deinit();
	protostack_deinit();
	return 0;
}


C_CAPSULE_END
