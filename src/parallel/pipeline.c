
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include <aroop/aroop_memory_profiler.h>
#include "nginez_config.h"
#include "plugin_manager.h"
#include "event_loop.h"
#include "parallel/pipeline.h"

C_CAPSULE_START

int child = -1;
int parent = -1;

NGINEZ_INLINE int pp_ping(aroop_txt_t*pkt) {
	// sanity check
	if(child == -1)
		return -1;
	
	sendto(child, aroop_txt_to_string(pkt), aroop_txt_length(pkt));
	return 0;
}

NGINEZ_INLINE int pp_pong(aroop_txt_t*pkt) {
	if(parent == -1)
		return -1;
	sendto(parent, aroop_txt_to_string(pkt), aroop_txt_length(pkt));
	return 0;
}

static int pipefd[2];
static int pp_fork_before_callback(aroop_txt_t*input, aroop_txt_t*output) {
	if(pipe2(&pipefd)) {
		return -1;
	}
	return 0;
}

static aroop_txt_t buffer;
static int on_ping(int events) {
	int count = recv(parent, aroop_txt_to_string(&buffer), aroop_txt_capacity(&buffer));
	// TODO do something with the buffer
}

static int on_pong(int events) {
	int count = recv(child, aroop_txt_to_string(&buffer), aroop_txt_capacity(&buffer));
	// TODO do something with the buffer
}

static int pp_fork_child_after_callback(aroop_txt_t*input, aroop_txt_t*output) {
	parent = pipefd[1];
	event_loop_register_fd(parent, on_ping, POLLIN);
	child = -1;
	return 0;
}

static int pp_fork_parent_after_callback(aroop_txt_t*input, aroop_txt_t*output) {
	child = pipefd[0];
	event_loop_register_fd(child, on_pong, POLLIN);
	return 0;
}

static int pp_fork_callback_desc(aroop_txt_t*plugin_space,aroop_txt_t*output) {
	return plugin_desc(output, "pipeline", "fork", plugin_space, __FILE__, "It allows the processes to pipeline messages to and forth.\n");
}

int pp_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/before");
	pm_plug_callback(&plugin_space, pp_fork_before_callback, pp_fork_callback_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/child/after");
	pm_plug_callback(&plugin_space, pp_fork_child_after_callback, pp_fork_callback_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/parent/after");
	pm_plug_callback(&plugin_space, pp_fork_parent_after_callback, pp_fork_callback_desc);
}

int pp_module_deinit() {
	// TODO unregister all
}

C_CAPSULE_END
