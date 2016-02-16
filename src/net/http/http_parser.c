
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
#include "net/http.h"
#include "net/http/http_parser.h"

C_CAPSULE_START

static struct http_hooks*hooks = NULL;
static int http_response_test_and_close(struct http_connection*http) {
	aroop_txt_t test = {};
	aroop_txt_embeded_set_static_string(&test, "HTTP/1.0 200 OK\r\nContent-Length: 9\r\n\r\nIt Works!\r\n");
	http->state = HTTP_QUIT;
	send(http->fd, aroop_txt_to_string(&test), aroop_txt_length(&test), 0);
	hooks->on_destroy(&http);
	return -1;
}

static int http_url_parse(aroop_txt_t*user_data, aroop_txt_t*target_url) {
	// TODO parse the headers
	return 0;
}

static aroop_txt_t recv_buffer = {};
static int http_on_client_data(int fd, int status, const void*cb_data) {
	struct http_connection*http = (struct http_connection*)cb_data;
	aroop_assert(http->fd == fd);
	aroop_txt_set_length(&recv_buffer, 1); // without it aroop_txt_to_string() will give NULL
	int count = recv(http->fd, aroop_txt_to_string(&recv_buffer), aroop_txt_capacity(&recv_buffer), 0);
	if(count == 0) {
		syslog(LOG_INFO, "Client disconnected\n");
		hooks->on_destroy(&http);
		return -1;
	}
	if(count >= NGINZ_MAX_HTTP_MSG_SIZE) {
		syslog(LOG_INFO, "Disconnecting HTTP client for too big data input\n");
		hooks->on_destroy(&http);
		return -1;
	}
	aroop_txt_set_length(&recv_buffer, count);
	//return x.processPacket(pkt);
	aroop_txt_t url = {};
	int response = http_url_parse(&recv_buffer, &url);
	if(response == 0) {
		// notify the page
		return http_response_test_and_close(http);
	}
	return response;
}

static int http_parser_hookup(int signature, void*given) {
	hooks = (struct http_hooks*)given;
	hooks->on_client_data = http_on_client_data;
	aroop_assert(hooks != NULL);
	return 0;
}

static int http_parser_hookup_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "http_parser", "http hooking", plugin_space, __FILE__, "It copies the hooks for future use.\n");
}

int http_parser_module_init() {
	// setup the hooks
	aroop_txt_embeded_buffer(&recv_buffer, NGINZ_MAX_HTTP_MSG_SIZE);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "httpproto/hookup");
	pm_plug_bridge(&plugin_space, http_parser_hookup, http_parser_hookup_desc);
}

int http_parser_module_deinit() {
	pm_unplug_bridge(0, http_parser_hookup);
	aroop_txt_destroy(&recv_buffer);
}


C_CAPSULE_END
