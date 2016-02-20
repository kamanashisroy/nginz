
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
#include "net/http/http_factory.h"

C_CAPSULE_START


static int default_http_close(struct streamio*strm) {
	if(strm->next) {
		return strm->next->close(strm->next);
	}
	if(strm->fd != INVALID_FD) {
		struct http_connection*http = (struct http_connection*)strm;
		event_loop_unregister_fd(strm->fd);
		if(!(http->state & HTTP_SOFT_QUIT))close(strm->fd);
		strm->fd = -1;
	}
	return 0;
}

static struct opp_factory http_factory;
static int http_factory_on_softquit(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	// iterate through all
	struct opp_iterator iterator = {};
	opp_iterator_create(&iterator, &http_factory, OPPN_ALL, 0, 0);
	struct http_connection*http = NULL;
	while(http = opp_iterator_next(&iterator)) {
		http->strm.close(&http->strm);
	}
	opp_iterator_destroy(&iterator);
	return 0;
}

static int http_factory_on_softquit_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "softquitall", "shake", plugin_space, __FILE__, "It tries to destroy all the connections.\n");
}

OPP_CB(http_connection) {
	struct http_connection*http = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			http->state = HTTP_CONNECTED;
			http->request = NULL;
			http->strm.send = default_streamio_send;
			http->strm.close = default_http_close;
			http->strm.next = NULL;
			http->strm.fd = -1;
		break;
		case OPPN_ACTION_FINALIZE:
			http->strm.close(&http->strm);
			http->strm.fd = -1;
		break;
	}
	return 0;
}


static struct http_connection*http_alloc(int fd) {
	struct http_connection*http = OPP_ALLOC1(&http_factory);
	http->strm.fd = fd;
	return http;
}

static int http_factory_hookup(int signature, void*given) {
	struct http_hooks*hooks = (struct http_hooks*)given;
	aroop_assert(hooks != NULL);
	hooks->on_create = http_alloc;
	return 0;
}

static int http_factory_hookup_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "http_factory", "http hooking", plugin_space, __FILE__, "It registers connection creation and destruction hooks.\n");
}


int http_factory_module_init() {
	NGINZ_FACTORY_CREATE(&http_factory, 64, sizeof(struct http_connection), OPP_CB_FUNC(http_connection));
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/softquitall");
	pm_plug_callback(&plugin_space, http_factory_on_softquit, http_factory_on_softquit_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "httpproto/hookup");
	pm_plug_bridge(&plugin_space, http_factory_hookup, http_factory_hookup_desc);
}

int http_factory_module_deinit() {
	pm_unplug_callback(0, http_factory_on_softquit);
	pm_unplug_bridge(0, http_factory_hookup);
	struct opp_iterator iterator;
	opp_iterator_create(&iterator, &http_factory, OPPN_ALL, 0, 0);
	do {
		struct http_connection*http = opp_iterator_next(&iterator);
		if(http == NULL)
			break;
		http->strm.close(&http->strm);
	} while(1);
	opp_iterator_destroy(&iterator);
	OPP_PFACTORY_DESTROY(&http_factory);
	return 0;
}

C_CAPSULE_END
