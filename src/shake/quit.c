
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include <aroop/aroop_memory_profiler.h>
#include "plugin.h"
#include "plugin_manager.h"
#include "shake/quit.h"

C_CAPSULE_START

static int shake_quit_command(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_t bin = {};
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset(&bin);
	aroop_txt_t quit_command = {};
	aroop_txt_embeded_set_static_string(&quit_command, "shake/quit"); 
	binary_pack_string(&bin, &quit_command);
	pp_ping(&bin);
	fiber_quit();
	return 0;
}

static int shake_quit_command_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "quit", "shake", plugin_space, __FILE__, "It stops the fibers and sends quit msg to other processes\n");
}


int shake_quit_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/quit");
	pm_plug_callback(&plugin_space, shake_quit_command, shake_quit_command_desc);
	return 0;
}


int shake_quit_module_deinit() {
	pm_unplug_callback(0, shake_quit_command);
}

C_CAPSULE_END

