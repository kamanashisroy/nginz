
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/streamio.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/chat_factory.h"

C_CAPSULE_START



static int default_chat_close(struct streamio*strm) {
	struct chat_connection*chat = (struct chat_connection*)strm;
	if(!(chat->state & (CHAT_SOFT_QUIT | CHAT_QUIT))) {
		chat->state |= CHAT_QUIT;
	}
	if(chat->state & CHAT_IN_ROOM) {
		broadcast_room_leave(chat);
	}
	if(chat->state & CHAT_LOGGED_IN) {
		logoff_user(chat);
	}
	if(strm->bubble_up) {
		return strm->bubble_up->close(strm->bubble_up);
	}
	if(strm->fd != INVALID_FD) {
		struct chat_connection*chat = (struct chat_connection*)strm;
		event_loop_unregister_fd(strm->fd);
		if(!(chat->state & CHAT_SOFT_QUIT))close(strm->fd);
		strm->fd = -1;
	}
	return 0;
}

static struct opp_factory chat_factory;

static int chat_factory_on_softquit(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	// iterate through all
	struct opp_iterator iterator = {};
	opp_iterator_create(&iterator, &chat_factory, OPPN_ALL, 0, 0);
	struct chat_connection*chat = NULL;
	while(chat = opp_iterator_next(&iterator)) {
		broadcast_room_leave(chat); // leave the room
		chat->strm.close(&chat->strm);
	}
	opp_iterator_destroy(&iterator);
	return 0;
}

static int chat_factory_on_softquit_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "softquitall", "shake", plugin_space, __FILE__, "It tries to destroy all the connections.\n");
}

static int chat_state_show(struct chat_connection*chat, aroop_txt_t*output) {
	aroop_txt_concat_char(output, '[');
	if(chat->state & CHAT_ZOMBIE) {
		aroop_txt_concat_string(output, "CHAT_ZOMBIE");
	} else if(chat->state & CHAT_QUIT) {
		aroop_txt_concat_string(output, "CHAT_QUIT");
	} else if(chat->state & CHAT_SOFT_QUIT) {
		aroop_txt_concat_string(output, "CHAT_SOFT_QUIT");
	} else if(chat->state & CHAT_IN_ROOM) {
		aroop_txt_concat_string(output, "CHAT_IN_ROOM");
	} else if(chat->state & CHAT_LOGGED_IN) {
		aroop_txt_concat_string(output, "CHAT_LOGGED_IN");
	} else {
		aroop_txt_concat_string(output, "CHAT_CONNECTED");
	}
	aroop_txt_concat_char(output, ']');
	return 0;
}

static int chat_factory_show(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(!IS_VALID_CHAT(chat)) // sanity check
		return 0;
	aroop_txt_t output = {};
	aroop_txt_embeded_stackbuffer(&output, 1024);
	struct opp_iterator iterator = {};
	opp_iterator_create(&iterator, &chat_factory, OPPN_ALL, 0, 0);
	struct chat_connection*other = NULL;
	while(other = opp_iterator_next(&iterator)) {
		aroop_txt_concat_char(&output, '\t');
		aroop_txt_concat(&output, &other->name);
		aroop_txt_concat_char(&output, '\t');
		chat_state_show(other, &output);
		aroop_txt_concat_char(&output, '\t');
		if(other->strm.bubble_up) {
			aroop_txt_concat_string(&output, "chained");
		}
		aroop_txt_concat_char(&output, '\t');
		if(other->strm.fd == INVALID_FD) {
			aroop_txt_concat_string(&output, "invalid fd");
		}
		aroop_txt_concat_char(&output, '\t');
		aroop_txt_concat_char(&output, '\n');
	}
	opp_iterator_destroy(&iterator);
	chat->strm.send(&chat->strm, &output, 0);
	return 0;
}

static int chat_factory_show_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "show", "chat", plugin_space, __FILE__, "It shows the chat objects.\n");
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
			streamio_initialize(&chat->strm);
			chat->strm.close = default_chat_close;
			chat->strm.on_recv = chat_api_get()->handle_chat_request;
		break;
		case OPPN_ACTION_FINALIZE:
			streamio_finalize(&chat->strm);
			aroop_txt_destroy(&chat->name);
		break;
	}
	return 0;
}

static struct chat_connection*chat_alloc(int fd) {
	struct chat_connection*chat = OPP_ALLOC1(&chat_factory);
	OPPREF(chat); // do not really destroy it ..
	chat->strm.fd = fd;
	return chat;
}

static struct chat_connection*chat_get(int token) {
	struct chat_connection*chat = opp_get(&chat_factory, token);
	if(chat->state & (CHAT_QUIT | CHAT_SOFT_QUIT | CHAT_ZOMBIE))
		return NULL;
	return chat;
}

int chat_factory_module_init() {
	NGINZ_EXTENDED_FACTORY_CREATE(&chat_factory, 64, sizeof(struct chat_connection), OPP_CB_FUNC(chat_connection));
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/show");
	composite_plug_bridge(chat_plugin_manager_get(), &plugin_space, chat_factory_show, chat_factory_show_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/softquitall");
	pm_plug_callback(&plugin_space, chat_factory_on_softquit, chat_factory_on_softquit_desc);
	chat_api_get()->on_create = chat_alloc;
	chat_api_get()->get = chat_get;
	return 0;
}

int chat_factory_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_factory_show);
	pm_unplug_callback(0, chat_factory_on_softquit);
	struct opp_iterator iterator;
	opp_iterator_create(&iterator, &chat_factory, OPPN_ALL, 0, 0);
	do {
		struct chat_connection*chat = opp_iterator_next(&iterator);
		if(chat == NULL)
			break;
		chat->strm.close(&chat->strm);
	} while(1);
	opp_iterator_destroy(&iterator);
	OPP_PFACTORY_DESTROY(&chat_factory);
	return 0;
}

C_CAPSULE_END
