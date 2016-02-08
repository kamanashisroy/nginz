
#include "aroop/aroop_core.h"
#include "aroop/core/xtring.h"
#include "inc/plugin_manager.h"
#include "inc/fiber.h"
#include "inc/event_loop.h"

C_CAPSULE_START

static int nginez_main(char*args) {
	pm_init();
	fiber_module_init();
	shake_module_init();
	event_loop_module_init();
	pp_module_init();
	fiber_module_run();
	pp_module_deinit();
	event_loop_module_deinit();
	shake_module_deinit();
	fiber_module_deinit();
	pm_deinit();
	return 0;
}

int main(int argc, char**argv) {
	aroop_main1(argc, argv, nginez_main);
}

C_CAPSULE_END
