
#include "aroop/aroop_core.h"
#include "aroop/core/xtring.h"
#include "nginz_config.h"
#include "log.h"
#include <signal.h>
#include "plugin_manager.h"
#include "fiber.h"
#include "fiber_internal.h"
#include "fork.h"
#include "db.h"
#include "shake/quitall.h"
#include "event_loop.h"
#include "inc/core_subsystem.h"

C_CAPSULE_START

static int nginz_main(char**args) {
	//daemon(0,0);
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("nginz", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	nginz_core_init();
	nginz_parallel_init();
	fiber_module_run();
	nginz_core_deinit();
	closelog();
	return 0;
}

int main(int argc, char**argv) {
	aroop_main1(argc, argv, nginz_main);
}

C_CAPSULE_END
