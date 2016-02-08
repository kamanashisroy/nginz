
#include "aroop/aroop_core.h"
#include "aroop/core/xtring.h"
#include "inc/plugin.h"
#include "inc/plugin_manager.h"
#include "inc/fiber.h"

C_CAPSULE_START



static struct composite_plugin*coreplug = NULL;
int pm_call(aroop_txt_t*plugin_space, aroop_txt_t*input, aroop_txt_t*output) {
	aroop_assert(coreplug != NULL);
	composite_plugin_call(coreplug, plugin_space, input, output);
}

struct composite_plugin*pm_get() {
	return coreplug;
}



int pm_init() {
	plugin_init();
	aroop_assert(coreplug == NULL);
	coreplug = composite_plugin_create();
}

int pm_deinit() {
	composite_plugin_destroy(coreplug);
	plugin_deinit();
}

C_CAPSULE_END
