
#include "aroop/aroop_core.h"
#include "aroop/core/xtring.h"
#include "nginz_config.h"
#include "log.h"
#include "plugin_manager.h"
#include "fiber.h"
#include "fork.h"
#include "db.h"
#include "shake/quitall.h"
#include "net/chat.h"
#include "event_loop.h"

C_CAPSULE_START

static int nginz_main(char*args) {
	daemon(0,0);
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("nginz", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	db_module_init();
	pm_init();
	shake_quitall_module_init();
	binary_coder_module_init();
	fiber_module_init();
	event_loop_module_init();
	protostack_init();
	chat_module_init();
	http_module_init();
	pp_module_init();
	fork_processors(NGINZ_NUMBER_OF_PROCESSORS);
	/**
	 * Setup for master
	 */
	shake_module_init(); // we start the control fd after fork 
	tcp_listener_init();
	/**
	 * master initialization complete ..
	 */
	fiber_module_run();
	tcp_listener_deinit();
	pp_module_deinit();
	http_module_deinit();
	chat_module_deinit();
	protostack_deinit();
	event_loop_module_deinit();
	shake_module_deinit();
	fiber_module_deinit();
	binary_coder_module_deinit();
	shake_quitall_module_deinit();
	pm_deinit();
	db_module_deinit();
	closelog();
	return 0;
}

int main(int argc, char**argv) {
	aroop_main1(argc, argv, nginz_main);
}

C_CAPSULE_END
