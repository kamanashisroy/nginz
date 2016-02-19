
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
#include "apps/web_chat/web_chat.h"

C_CAPSULE_START



static struct opp_factory web_chat_factory;
static int web_chat_factory_on_softquit(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	// iterate through all
	struct opp_iterator iterator = {};
	opp_iterator_create(&iterator, &web_chat_factory, OPPN_ALL, 0, 0);
	struct web_chat_connection*web_chat = NULL;
	while(web_chat = opp_iterator_next(&iterator)) {
		event_loop_unregister_fd(web_chat->fd);
		if(web_chat->fd != -1)close(web_chat->fd);
		web_chat->fd = -1;
	}
	opp_iterator_destroy(&iterator);
	return 0;
}

static int web_chat_factory_on_softquit_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "softquitall", "shake", plugin_space, __FILE__, "It tries to destroy all the connections.\n");
}

OPP_CB(web_chat_connection) {
	struct web_chat_connection*web_chat = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			web_chat->http.state = HTTP_CONNECTED;
			web_chat->http.request = NULL;
		break;
		case OPPN_ACTION_FINALIZE:
			// cleanup socket
			event_loop_unregister_fd(web_chat->http.fd);
			if(web_chat->http.fd != -1 && web_chat->http.state != HTTP_SOFT_QUIT)close(web_chat->http.fd);
			web_chat->http.fd = -1;
			if(web_chat->chat) {
				web_chat->chat->fd = -1;
				OPPUNREF(web_chat->chat);
			}
		break;
	}
	return 0;
}

static struct web_chat_connection*web_chat_alloc(int fd) {
	struct web_chat_connection*web_chat = OPP_ALLOC1(&web_chat_factory);
	web_chat->fd = fd;
	return web_chat;
}

static int web_chat_factory_hookup(int signature, void*given) {
	struct web_chat_hooks*hooks = (struct web_chat_hooks*)given;
	aroop_assert(hooks != NULL);
	hooks->on_create = web_chat_alloc;
	return 0;
}

static int web_chat_factory_hookup_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "web_chat_factory", "web_chat hooking", plugin_space, __FILE__, "It registers connection creation and destruction hooks.\n");
}


int web_chat_factory_module_init() {
	NGINZ_FACTORY_CREATE(&web_chat_factory, 64, sizeof(struct web_chat_connection), OPP_CB_FUNC(web_chat_connection));
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/softquitall");
	pm_plug_callback(&plugin_space, web_chat_factory_on_softquit, web_chat_factory_on_softquit_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "web_chatproto/hookup");
	pm_plug_bridge(&plugin_space, web_chat_factory_hookup, web_chat_factory_hookup_desc);
}

int web_chat_factory_module_deinit() {
	pm_unplug_callback(0, web_chat_factory_on_softquit);
	pm_unplug_bridge(0, web_chat_factory_hookup);
	struct opp_iterator iterator;
	opp_iterator_create(&iterator, &web_chat_factory, OPPN_ALL, 0, 0);
	do {
		struct web_chat_connection*web_chat = opp_iterator_next(&iterator);
		if(web_chat == NULL)
			break;
		event_loop_unregister_fd(web_chat->fd);
		if(web_chat->fd != -1)close(web_chat->fd);
		web_chat->fd = -1;
	} while(1);
	opp_iterator_destroy(&iterator);
	OPP_PFACTORY_DESTROY(&web_chat_factory);
	return 0;
}

C_CAPSULE_END
