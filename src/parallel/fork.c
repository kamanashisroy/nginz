#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include "nginz_config.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "fork.h"



C_CAPSULE_START

int fork_processors(int nclients) {
	if(nclients == 0)
		return 0;
	nclients--;
	pid_t pid = 0;
	aroop_txt_t input = {};
	aroop_txt_t output = {};
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/before");
	pm_call(&plugin_space, &input, &output);
	aroop_txt_destroy(&output);
	pid = fork();
	if(pid > 0) { // parent
		aroop_txt_embeded_set_static_string(&plugin_space, "fork/parent/after");
		pm_call(&plugin_space, &input, &output);
		aroop_txt_destroy(&output);
	} else if(pid == 0) { // child
		aroop_txt_embeded_set_static_string(&plugin_space, "fork/child/after");
		pm_call(&plugin_space, &input, &output);
		aroop_txt_destroy(&output);
		fork_processors(nclients); // fork more
	} else {
		//perror();
		return -1;
	}
	return 0;
}

C_CAPSULE_END

