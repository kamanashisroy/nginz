
#include "aroop/aroop_core.h"
#include "aroop/core/thread.h"
#include "aroop/core/xtring.h"
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "aroop/opp/opp_any_obj.h"
#include "aroop/opp/opp_str2.h"
#include "aroop/aroop_memory_profiler.h"
#include "nginez_config.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "event_loop.h"
#include "shake.h"

C_CAPSULE_START

static int on_shake_command(int events) {
	// read data from stdin
	char mem[128]; 
	char*cmd = fgets(mem, 128, stdin);
	if(!cmd)
		return 0;
	aroop_txt_t xcmd;
	aroop_txt_embeded_set_content(&xcmd, cmd, strlen(cmd), NULL);
	aroop_txt_t target = {};
	shotodol_scanner_next_token(&xcmd, &target);
	if(aroop_txt_length(&target) == 0)
		return 0;
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_stackbuffer(&plugin_space, 64);
	aroop_txt_concat_string(&plugin_space, "shake/");
	aroop_txt_concat(&plugin_space, &target);
	aroop_txt_t input = {};
	aroop_txt_t output = {};
	pm_call(&plugin_space, &input, &output);
	aroop_txt_zero_terminate(&output);
	printf("%s", aroop_txt_to_string(&output));
	aroop_txt_destroy(&output);
	return 0;
}

static int shake_stop_on_fork(aroop_txt_t*input, aroop_txt_t*output) {
	//close(stdin);
	event_loop_unregister_fd(STDIN_FILENO);
}

static int shake_stop_on_fork_desc(aroop_txt_t*plugin_space,aroop_txt_t*output) {
	return plugin_desc(output, "shake", "fork", plugin_space, __FILE__, "It stops stdio in child process\n");
}



int shake_module_init() {
	aroop_txt_t plugin_space = {};
	// register shake shell
	event_loop_register_fd(STDIN_FILENO, on_shake_command, NGINEZ_POLL_ALL_FLAGS);
	help_module_init();
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/child/after");
	pm_plug_callback(&plugin_space, shake_stop_on_fork, shake_stop_on_fork_desc);
}

int shake_module_deinit() {
	event_loop_unregister_fd(STDIN_FILENO);
	help_module_deinit();
}

C_CAPSULE_END

