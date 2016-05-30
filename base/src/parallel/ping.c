
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_str2.h>
#include "nginz_config.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "binary_coder.h"
#include "parallel/pipeline.h"
#include "parallel/ping.h"
#include "scanner.h"

C_CAPSULE_START

aroop_txt_t ping_buffer = {};
static int ping_command(aroop_txt_t*input, aroop_txt_t*output) {

	aroop_txt_t cmd = {};
	scanner_trim(input, &cmd);

	binary_coder_reset_for_pid(&ping_buffer, getpid());
	binary_pack_string(&ping_buffer, &cmd);
	aroop_txt_embeded_buffer(output, 64);
	if(pp_send(0, &ping_buffer)) {
		aroop_txt_printf(output, "ping[%s] failed\n", aroop_txt_to_string(&cmd)); // XXX the sandbox may not be zero terminated, but sandbox is readonly ..
	}
	aroop_txt_printf(output, "ping[%s]\n", aroop_txt_to_string(&cmd)); // XXX the sandbox may not be zero terminated, but sandbox is readonly ..
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
	return 0;
}

int ping_module_deinit() {
	pm_unplug_callback(0, ping_command);
	aroop_txt_destroy(&ping_buffer);
	return 0;
}

C_CAPSULE_END
