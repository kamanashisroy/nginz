
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
#include "event_loop.h"
#include "async_db_internal.h"
#include "db_subsystem.h"

C_CAPSULE_START

int nginz_db_module_init_after_parallel_init() {
	async_db_init();
	return 0;
}

int nginz_db_module_deinit() {
	async_db_deinit();
	return 0;
}


C_CAPSULE_END
