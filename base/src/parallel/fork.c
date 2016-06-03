#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include "nginz_config.h"
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "parallel/pipeline.h"
#include "fork.h"


C_CAPSULE_START

int fork_processors(int nclients) {
	if(nclients == 0)
		return 0;
	nclients--;
	pid_t pid = 0;
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/before");
	pm_bridge_call(&plugin_space, NGINZ_PIPELINE_SIGNATURE, NULL);
	pid = fork();
	if(pid > 0) { // parent
		aroop_txt_embeded_set_static_string(&plugin_space, "fork/parent/after");
		pm_bridge_call(&plugin_space, NGINZ_PIPELINE_SIGNATURE, &pid);
		fork_processors(nclients); // fork more
	} else if(pid == 0) { // child
		aroop_txt_embeded_set_static_string(&plugin_space, "fork/child/after");
		pm_bridge_call(&plugin_space, NGINZ_PIPELINE_SIGNATURE, &pid);
	} else {
		syslog(LOG_ERR, "Failed to fork:%s\n", strerror(errno));
		return -1;
	}
	return 0;
}

C_CAPSULE_END

