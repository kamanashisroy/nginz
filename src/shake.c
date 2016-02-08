
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

#include <sys/ioctl.h> // defines FIONREAD 
#define fileio_available_bytes(x) ({ \
        int __bt=0;ioctl(x, FIONREAD, &__bt);__bt; \
})

static int shake_shell_step(int status) {
	if(fileio_available_bytes(STDIN_FILENO) == 0)
		return 0;
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
	return 0;
}

int shake_module_init() {
	// XXX why to get all the available commands from the plugin ? may be we do not need it ..
	
	// register shake shell
	register_fiber(shake_shell_step);
	help_module_init();
}

int shake_module_deinit() {
	unregister_fiber(shake_shell_step);
	help_module_deinit();
}
C_CAPSULE_END

