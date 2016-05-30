
#include "aroop/aroop_core.h"
#include "aroop/core/xtring.h"
#include "nginz_config.h"
#include "log.h"
#include <signal.h>
#include "plugin_manager.h"
#include "fiber.h"
#include "fiber_internal.h"
#include "fork.h"
#include "db.h"
#include "event_loop.h"
#include "binary_coder_internal.h"
#include "binary_coder_internal.h"
#include "parallel/pipeline.h"
#include "shake.h"
#include "shake/quitall.h"
#include "shake/shake_internal.h"
#include "base_subsystem.h"

C_CAPSULE_START

static int rehash() {
	aroop_txt_t input = {};
	aroop_txt_t output = {};
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/rehash");
	pm_call(&plugin_space, &input, &output);
	aroop_txt_destroy(&input);
	aroop_txt_destroy(&output);
	return 0;
}

static int master_init() {
	aroop_txt_t input = {};
	aroop_txt_t output = {};
	aroop_txt_t plugin_space = {};
	//TODO tcp_listener_init();
	aroop_txt_embeded_set_static_string(&plugin_space, "master/init");
	pm_call(&plugin_space, &input, &output);
	aroop_txt_destroy(&input);
	aroop_txt_destroy(&output);
	return 0;
}

static void signal_callback(int sigval) {
	fiber_quit();
}

int nginz_parallel_init() {
	rehash();
	signal(SIGPIPE, SIG_IGN); // avoid crash on sigpipe
	signal(SIGINT, signal_callback);
	fork_processors(NGINZ_NUMBER_OF_PROCESSORS);
	/**
	 * Setup for master
	 */
	shake_module_init(); // we start the control fd after fork 
	master_init();
	return 0;
}

static int initiated = 0;
int nginz_core_init() {
	if(initiated) /* already initiated */
		return 0;
	pm_init();
	pp_module_init();
	shake_quitall_module_init();
	binary_coder_module_init();
	fiber_module_init();
	event_loop_module_init();
	enumerate_module_init();
	initiated = 1;
	return 0;
}

int nginz_core_deinit() {
	enumerate_module_deinit();
	event_loop_module_deinit();
	shake_module_deinit();
	fiber_module_deinit();
	binary_coder_module_deinit();
	shake_quitall_module_deinit();
	pp_module_deinit();
	pm_deinit();
	return 0;
}


C_CAPSULE_END
