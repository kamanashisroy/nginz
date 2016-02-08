
#include "aroop/aroop_core.h"
#include "aroop/core/thread.h"
#include "aroop/core/xtring.h"
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "aroop/opp/opp_any_obj.h"
#include "aroop/opp/opp_str2.h"
#include "aroop/aroop_memory_profiler.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "shake.h"

C_CAPSULE_START

static int shake_help(aroop_txt_t*input, aroop_txt_t*output) {
	printf("TODO: we should show help\n");
}

int help_module_init() {
	aroop_txt_t help_plug;
	aroop_txt_embeded_set_static_string(&help_plug, "shake/help");
	pm_plug_callback(&help_plug, shake_help);
}

int help_module_deinit() {
	pm_unplug_callback(0, shake_help);
}

C_CAPSULE_END
