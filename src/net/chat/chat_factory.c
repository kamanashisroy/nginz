
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
#include "net/chat.h"
#include "net/chat/chat_factory.h"

C_CAPSULE_START



static struct opp_factory chat_factory;

static int chat_send_content(struct chat_connection*chat, aroop_txt_t*content, int flag) {
	return send(chat->fd, aroop_txt_to_string(content), aroop_txt_length(content), flag);
}

static int chat_factory_on_softquit(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	// iterate through all
	struct opp_iterator iterator = {};
	opp_iterator_create(&iterator, &chat_factory, OPPN_ALL, 0, 0);
	struct chat_connection*chat = NULL;
	while(chat = opp_iterator_next(&iterator)) {
		broadcast_room_leave(chat); // leave the room
		event_loop_unregister_fd(chat->fd);
		close(chat->fd);
		chat->fd = -1;
	}
	opp_iterator_destroy(&iterator);
	return 0;
}

static int chat_factory_on_softquit_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "softquitall", "shake", plugin_space, __FILE__, "It tries to destroy all the connections.\n");
}

OPP_CB(chat_connection) {
	struct chat_connection*chat = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			aroop_memclean_raw2(&chat->name);
			chat->on_response_callback = NULL;
			chat->callback_data = NULL;
			chat->state = CHAT_CONNECTED;
			chat->request = NULL;
			chat->send = chat_send_content;
		break;
		case OPPN_ACTION_FINALIZE:
			event_loop_unregister_fd(chat->fd);
			if(chat->fd != -1 && !(chat->state & CHAT_SOFT_QUIT))close(chat->fd);
			chat->fd = -1;
			aroop_txt_destroy(&chat->name);
		break;
	}
	return 0;
}

OPP_CB(chat_connection_vcall) {
	struct chat_connection*chat = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			chat->opp_cb = OPP_CB_FUNC(chat_connection); // dynamically send default object handler
		break;
	}
	return chat->opp_cb(data, callback, cb_data, ap, size);
}

static struct chat_connection*chat_alloc(int fd) {
	struct chat_connection*chat = OPP_ALLOC1(&chat_factory);
	chat->fd = fd;
	return chat;
}

static int chat_factory_hookup(int signature, void*given) {
	struct chat_hooks*hooks = (struct chat_hooks*)given;
	aroop_assert(hooks != NULL);
	hooks->on_create = chat_alloc;
	return 0;
}

static int chat_factory_hookup_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "chat_factory", "chat hooking", plugin_space, __FILE__, "It registers connection creation and destruction hooks.\n");
}


int chat_factory_module_init() {
	NGINZ_FACTORY_CREATE(&chat_factory, 64, sizeof(struct chat_connection), OPP_CB_FUNC(chat_connection_vcall));
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/softquitall");
	pm_plug_callback(&plugin_space, chat_factory_on_softquit, chat_factory_on_softquit_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "chatproto/hookup");
	pm_plug_bridge(&plugin_space, chat_factory_hookup, chat_factory_hookup_desc);
}

int chat_factory_module_deinit() {
	pm_unplug_callback(0, chat_factory_on_softquit);
	pm_unplug_bridge(0, chat_factory_hookup);
	struct opp_iterator iterator;
	opp_iterator_create(&iterator, &chat_factory, OPPN_ALL, 0, 0);
	do {
		struct chat_connection*chat = opp_iterator_next(&iterator);
		if(chat == NULL)
			break;
		event_loop_unregister_fd(chat->fd);
		if(chat->fd != -1)close(chat->fd);
		chat->fd = -1;
	} while(1);
	opp_iterator_destroy(&iterator);
	OPP_PFACTORY_DESTROY(&chat_factory);
	return 0;
}

C_CAPSULE_END