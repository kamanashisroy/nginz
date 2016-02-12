
#include "aroop/aroop_core.h"
#include "aroop/core/xtring.h"
#include "nginz_config.h"
#include "plugin_manager.h"
#include "fiber.h"
#include "fork.h"
#include "db.h"
#include "net/chat.h"
#include "event_loop.h"

C_CAPSULE_START

static int nginz_main(char*args) {
	db_module_init();
	pm_init();
	binary_coder_module_init();
	fiber_module_init();
	shake_module_init();
	event_loop_module_init();
	chat_module_init();
	pp_module_init();
	fork_processors(NGINZ_NUMBER_OF_PROCESSORS);
	tcp_listener_init();
	fiber_module_run();
	tcp_listener_deinit();
	pp_module_deinit();
	chat_module_deinit();
	event_loop_module_deinit();
	shake_module_deinit();
	fiber_module_deinit();
	binary_coder_module_deinit();
	pm_deinit();
	db_module_deinit();
	return 0;
}

int main(int argc, char**argv) {
	aroop_main1(argc, argv, nginz_main);
}

C_CAPSULE_END
