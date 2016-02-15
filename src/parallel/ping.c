
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "inc/parallel/pipeline.h"
#include "inc/parallel/ping.h"

C_CAPSULE_START

aroop_txt_t ping_buffer = {};
static int ping_command(aroop_txt_t*input, aroop_txt_t*output) {
	binary_coder_reset(&ping_buffer);
	binary_pack_string(&ping_buffer, input);
	pp_bubble_down(&ping_buffer);
	aroop_txt_embeded_buffer(output, 32);
	aroop_txt_printf(output, "ping[%s]\n", aroop_txt_to_string(input)); // XXX the sandbox may not be zero terminated, but sandbox is readonly ..
	return 0;
}

static int ping_command_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "ping", "shake", plugin_space, __FILE__, "It pings the child process. for example(ping quit) will quit all the child process\n");
}

int ping_module_init() {
	aroop_txt_embeded_buffer(&ping_buffer, 1024); // XXX number of rooms is limited by this 1024 size.
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/ping");
	pm_plug_callback(&plugin_space, ping_command, ping_command_desc);
}

int ping_module_deinit() {
	pm_unplug_callback(0, ping_command);
	aroop_txt_destroy(&ping_buffer);
}

C_CAPSULE_END
