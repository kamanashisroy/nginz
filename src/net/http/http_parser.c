
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
#include "net/http/http_parser.h"

C_CAPSULE_START

static struct http_hooks*hooks = NULL;
static int http_response_test_and_close(struct http_connection*http) {
	aroop_txt_t test = {};
	aroop_txt_embeded_set_static_string(&test, "HTTP/1.0 200 OK\r\nContent-Length: 9\r\n\r\nIt Works!\r\n");
	http->state = HTTP_QUIT;
	http->strm.send(&http->strm, &test, 0);
	http->strm.close(&http->strm);
	OPPUNREF(http);
	return -1;
}

static int http_url_go(struct http_connection*http, aroop_txt_t*target) {
	int ret = 0;
	int len = aroop_txt_length(target);
	if(aroop_txt_is_empty(target) || (len == 1))
		return http_response_test_and_close(http);
	http->is_processed = 0;
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_stackbuffer(&plugin_space, len+32);
	aroop_txt_concat_string(&plugin_space, "http");
	if(aroop_txt_char_at(target, 0) != '/') {
		aroop_txt_concat_char(&plugin_space, '/');
	}
	aroop_txt_concat(&plugin_space, target);
	ret = composite_plugin_bridge_call(http_plugin_manager_get(), &plugin_space, HTTP_SIGNATURE, http);
	if(!http->is_processed) { // if not processed
		// say not found
		aroop_txt_t not_found = {};
		aroop_txt_embeded_set_static_string(&not_found, "HTTP/1.0 404 NOT FOUND\r\nContent-Length: 9\r\n\r\nNot Found");
		http->strm.send(&http->strm, &not_found, 0);
	}
	return ret;
}

static int http_url_parse(aroop_txt_t*user_data, aroop_txt_t*target_url) {
	aroop_txt_zero_terminate(user_data);
	char*content = aroop_txt_to_string(user_data);
	char*prev_header = NULL;
	char*header = NULL;
	char*url = NULL;
	char*header_str = NULL;
	int header_len = 0;
	int skip_len = 0;
	int ret = 0;
	while((header = strchr(content, '\n'))) {
		// skip the new line
		header++;
		if(prev_header == NULL) {
			// it is the request string ..
			header_len = header-content;
			if(header_len > 256) { // too big header
				ret = -1;
				break;
			}
			aroop_txt_embeded_buffer(target_url, header_len);
			aroop_txt_concat_string_len(target_url, content, header_len);
			aroop_txt_zero_terminate(target_url);
			header_str = aroop_txt_to_string(target_url);
			url = strchr(header_str, ' '); // GET url HTTP/1.1
			if(!url) {
				ret = -1;
				break;
			}
			url++; // skip the space
			skip_len = url - header_str;
			aroop_txt_shift(target_url, skip_len);
			header_str = url;
			url = strchr(header_str, ' '); // url HTTP/1.1
			if(!url) {
				ret = -1;
				break;
			}
			skip_len = url - header_str;
			aroop_txt_truncate(target_url, skip_len);
			// we do not parse anymore
			break;
		}
		prev_header = header;
	}
	
	if(ret == -1)aroop_txt_destroy(target_url);
	return ret;
}

static aroop_txt_t recv_buffer = {};
static int http_on_client_data(int fd, int status, const void*cb_data) {
	struct http_connection*http = (struct http_connection*)cb_data;
	aroop_assert(http->strm.fd == fd);
	aroop_txt_set_length(&recv_buffer, 1); // without it aroop_txt_to_string() will give NULL
	int count = recv(http->strm.fd, aroop_txt_to_string(&recv_buffer), aroop_txt_capacity(&recv_buffer), 0);
	if(count == 0) {
		syslog(LOG_INFO, "Client disconnected\n");
		OPPUNREF(http);
		return -1;
	}
	if(count >= NGINZ_MAX_HTTP_MSG_SIZE) {
		syslog(LOG_INFO, "Disconnecting HTTP client for too big data input\n");
		OPPUNREF(http);
		return -1;
	}
	aroop_txt_set_length(&recv_buffer, count);
	//return x.processPacket(pkt);
	aroop_txt_t url = {};
	int response = http_url_parse(&recv_buffer, &url);
	if(response == 0) {
		// notify the page
		response = http_url_go(http, &url);
	}
	// cleanup
	aroop_txt_destroy(&url);
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
