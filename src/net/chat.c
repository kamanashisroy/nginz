
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/chat.h"

C_CAPSULE_START

static struct composite_plugin*chat_plug = NULL;

NGINZ_INLINE struct composite_plugin*chat_context_get() {
	return chat_plug;
}

static int chat_destroy(struct chat_connection*chat) {
	// cleanup socket
	event_loop_unregister_fd(chat->fd);
	if(chat->fd != -1)close(chat->fd);
	// free data
	OPPUNREF(chat);
}

static aroop_txt_t recv_buffer = {};
static int on_client_data(int status, const void*cb_data) {
	struct chat_connection*chat = (struct chat_connection*)cb_data;
	aroop_txt_set_length(&recv_buffer, 1); // without it aroop_txt_to_string() will give NULL
	int count = recv(chat->fd, aroop_txt_to_string(&recv_buffer), aroop_txt_capacity(&recv_buffer), 0);
	if(count == 0) {
		printf("Client disconnected\n");
		chat_destroy(chat);
		return -1;
	}
	aroop_txt_set_length(&recv_buffer, count);
	if(chat->on_answer != NULL) {
		chat->on_answer(chat, &recv_buffer);
	}
	// read user data
}

static struct opp_factory chat_factory;
static aroop_txt_t chat_welcome = {};
static int chat_on_connect(int fd) {
	struct chat_connection*chat = OPP_ALLOC1(&chat_factory);
	chat->fd = fd;
	composite_plugin_bridge_call(chat_context_get(), &chat_welcome, CHAT_SIGNATURE, chat);
	event_loop_register_fd(fd, on_client_data, chat, NGINZ_POLL_ALL_FLAGS);
	return 0;
}

struct protostack chat_protostack = {
	.on_connect = chat_on_connect,
};

OPP_CB(chat_connection) {
	struct chat_connection*chat = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			chat->on_answer = NULL;
		break;
		case OPPN_ACTION_FINALIZE:
		break;
	}
	return 0;
}



int chat_module_init() {
	OPP_PFACTORY_CREATE(&chat_factory, 64, sizeof(struct chat_connection), OPP_CB_FUNC(chat_connection));
	aroop_txt_embeded_set_static_string(&chat_welcome, "chat/welcome");
	aroop_txt_embeded_buffer(&recv_buffer, 255);
	aroop_assert(chat_plug == NULL);
	chat_plug = composite_plugin_create();
	protostack_set(NGINZ_DEFAULT_PORT, &chat_protostack);
	welcome_module_init();
}

int chat_module_deinit() {
	welcome_module_deinit();
	composite_plugin_destroy(chat_plug);
	aroop_txt_destroy(&recv_buffer);
	// TODO  unplug all the clients
	struct opp_iterator iterator;
	opp_iterator_create(&iterator, &chat_factory, OPPN_ALL, 0, 0);
	do {
		struct chat_connection*chat = opp_iterator_next(&iterator);
		if(chat == NULL)
			break;
		event_loop_unregister_fd(chat->fd);
		close(chat->fd);
		chat->fd = -1;
		//OPPUNREF(plugin);
	} while(1);
	opp_iterator_destroy(&iterator);
	OPP_PFACTORY_DESTROY(&chat_factory);
}


C_CAPSULE_END
