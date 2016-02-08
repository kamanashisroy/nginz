
#include "aroop/aroop_core.h"
#include "aroop/core/thread.h"
#include "aroop/core/xtring.h"
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "aroop/opp/opp_any_obj.h"
#include "aroop/opp/opp_str2.h"
#include "aroop/aroop_memory_profiler.h"
#include "plugin.h"
#include "shake.h"

C_CAPSULE_START
static int shake_shell_step(int status) {
	// read data from stdin
	char mem[128]; 
	char*cmd = fgets_unlocked(mem, 128, stdin);
	if(cmd == NULL)
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
	printf("We executed the commnad\n");
}

int shake_module_init() {
	// XXX why to get all the available commands from the plugin ? may be we do not need it ..
	
	// register shake shell
	register_fiber(shake_shell_step);
}

int shake_module_deinit() {
	unregister_fiber(shake_shell_step);
}
C_CAPSULE_END

