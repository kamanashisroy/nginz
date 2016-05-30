
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_str2.h>
#include "nginz_config.h"
#include "plugin.h"
#include "plugin_manager.h"

C_CAPSULE_START

long long count = 0;
static int enumerate_command(aroop_txt_t*input, aroop_txt_t*output) {
	count++;
	aroop_txt_embeded_buffer(output, 32);
	aroop_txt_printf(output, "-> [%lld]\n", count);
	return 0;
}

static int enumerate_command_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "enumerate", "shake", plugin_space, __FILE__, "It increments an internal counter and displays it.\n");
}

int enumerate_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/enumerate");
	pm_plug_callback(&plugin_space, enumerate_command, enumerate_command_desc);
	return 0;
}

int enumerate_module_deinit() {
	pm_unplug_callback(0, enumerate_command);
	return 0;
}

C_CAPSULE_END
