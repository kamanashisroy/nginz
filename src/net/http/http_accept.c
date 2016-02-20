
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "plugin.h"
#include "log.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/streamio.h"
#include "net/http.h"
#include "net/http/http_accept.h"

C_CAPSULE_START

static struct http_hooks*hooks = NULL;
int http_accept_module_is_quiting = 0;
static int http_on_tcp_connection(int fd) {
	aroop_txt_t bin = {};
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset_for_pid(&bin, 0);
	binary_pack_int(&bin, NGINZ_HTTP_PORT);
	aroop_txt_t welcome_command = {};
	aroop_txt_embeded_set_static_string(&welcome_command, "http/welcome"); 
	binary_pack_string(&bin, &welcome_command);
	pp_bubble_down_send_socket(fd, &bin);
	return 0;
}

#ifdef NGINZ_EVENT_DEBUG
static int on_http_debug(int fd, const void*cb_data) {
	struct http_connection*http = (struct http_connection*)cb_data;
	aroop_assert(http->strm.fd == fd);
	return 0;
}
#endif

static struct opp_factory http_factory;
static int http_on_connection_bubble(int fd, aroop_txt_t*cmd) {
	aroop_assert(hooks != NULL);
	if(http_accept_module_is_quiting)
		return 0;
	// parse the http header
	aroop_txt_t request = {};
	binary_unpack_string(cmd, 2, &request); // needs cleanup

	struct http_connection*http = hooks->on_create(fd);
	if(!http) {
		syslog(LOG_ERR, "Could not create http object\n");
		close(fd);
		aroop_txt_destroy(&request); // cleanup
		return -1;
	}

	int ret = 0;
	// get the target command
	aroop_txt_t plugin_space = {};
	int reqlen = aroop_txt_length(&request);
	aroop_txt_t request_sandbox = {};
	aroop_txt_embeded_stackbuffer(&request_sandbox, reqlen);
	aroop_txt_concat(&request_sandbox, &request);
	shotodol_scanner_next_token(&request_sandbox, &plugin_space); // needs cleanup

	do {
		if(aroop_txt_is_empty(&plugin_space)) {
			aroop_txt_zero_terminate(&request_sandbox);
			syslog(LOG_ERR, "Possible BUG , cannot handle request %s", aroop_txt_to_string(&request_sandbox));
			OPPUNREF(http);
			break;
		}

		aroop_assert(http);
		aroop_assert(http->strm.fd == fd);
		// register it in the event loop
		event_loop_register_fd(fd, hooks->on_client_data, http, NGINZ_POLL_ALL_FLAGS);
#ifdef NGINZ_EVENT_DEBUG
		event_loop_register_debug(fd, on_http_debug);
#endif

#if 0 // do nothing for http/welcome
		// execute the command
		composite_plugin_bridge_call(http_plugin_manager_get(), &plugin_space, HTTP_SIGNATURE, http);
#endif
	} while(0);
	aroop_txt_destroy(&request); // cleanup
	aroop_txt_destroy(&request_sandbox); // cleanup
	aroop_txt_destroy(&plugin_space); // cleanup
	return ret;
}

static struct protostack http_protostack = {
	.on_tcp_connection = http_on_tcp_connection,
	.on_connection_bubble = http_on_connection_bubble,
};

static int http_accept_hookup(int signature, void*given) {
	hooks = (struct http_hooks*)given;
	aroop_assert(hooks != NULL);
	return 0;
}

static int http_accept_hookup_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "http_accept", "http hooking", plugin_space, __FILE__, "It copies the hooks for future use.\n");
}

int http_accept_module_init() {
	protostack_set(NGINZ_HTTP_PORT, &http_protostack);
	
	// setup the hooks
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "httpproto/hookup");
	pm_plug_bridge(&plugin_space, http_accept_hookup, http_accept_hookup_desc);
}

int http_accept_module_deinit() {
	protostack_set(NGINZ_HTTP_PORT, NULL);
	pm_unplug_bridge(0, http_accept_hookup);
}


C_CAPSULE_END
