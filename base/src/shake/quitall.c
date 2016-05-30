
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include <aroop/aroop_memory_profiler.h>
#include "nginz_config.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "binary_coder.h"
#include "shake/quitall.h"
#include "parallel/pipeline.h"
#include "fiber_internal.h"

C_CAPSULE_START

static int soft_quitall = 0;
static int shake_soft_quitall_command(aroop_txt_t*input, aroop_txt_t*output) {
	if(soft_quitall) /* soft quit is already complete */
		return 0;
	int next = pp_next_nid();
	if(next != -1) { // propagate the command
		aroop_txt_embeded_rebuild_and_set_static_string(output, "Sending softquit to all\n");
		aroop_txt_t bin = {};
		aroop_txt_embeded_stackbuffer(&bin, 255);
		binary_coder_reset(&bin);
		aroop_txt_t quitall_command = {};
		aroop_txt_embeded_set_static_string(&quitall_command, "shake/softquitall"); 
		binary_pack_int(&bin, getpid());
		binary_pack_string(&bin, &quitall_command);
		pp_send(next, &bin);
	}
	soft_quitall = 1;
	return 0;
}

static int shake_quitall_command(aroop_txt_t*input, aroop_txt_t*output) {
	if(!soft_quitall) {
		aroop_txt_embeded_rebuild_and_set_static_string(output, "Please softquitall first\n");
		return 0;
	}
	int next = pp_next_nid();
	if(next != -1) { // propagate the command
		aroop_txt_embeded_rebuild_and_set_static_string(output, "Sending QUIT to all\n");
		aroop_txt_t bin = {};
		aroop_txt_embeded_stackbuffer(&bin, 255);
		binary_coder_reset(&bin);
		binary_pack_int(&bin, getpid());
		aroop_txt_t quitall_command = {};
		aroop_txt_embeded_set_static_string(&quitall_command, "shake/quitall"); 
		binary_pack_string(&bin, &quitall_command);
		pp_send(next, &bin);
	}
	fiber_quit();
	return 0;
}

static int shake_quitall_command_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "quitall", "shake", plugin_space, __FILE__, "It stops the fibers and sends quitall msg to other processes\n");
}


int shake_quitall_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/quitall");
	pm_plug_callback(&plugin_space, shake_quitall_command, shake_quitall_command_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/softquitall");
	pm_plug_callback(&plugin_space, shake_soft_quitall_command, shake_quitall_command_desc);
	return 0;
}


int shake_quitall_module_deinit() {
	pm_unplug_callback(0, shake_quitall_command);
	pm_unplug_callback(0, shake_soft_quitall_command);
	return 0;
}

C_CAPSULE_END

