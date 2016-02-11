
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_iterator.h>
#include <aroop/opp/opp_list.h>
#include "nginz_config.h"
#include "plugin.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/broadcast.h"

C_CAPSULE_START

struct internal_room {
	struct opp_object_ext _ext;
	aroop_txt_t name;
	struct opp_factory user_list;
};
static struct opp_factory room_factory;

static int broadcast_callback(struct chat_connection*chat, aroop_txt_t*msg) {
	struct internal_room*rm = (struct internal_room*)(chat->broadcast_data);
	if(!rm) {
		printf("We do not know how we can broadcast it\n");
		return -1;
	}
	printf("found room to broadcast %s\n", aroop_txt_to_string(&rm->name));
	struct opp_iterator iterator = {};
	opp_iterator_create(&iterator, &rm->user_list, OPPN_ALL, 0, 0);
	opp_pointer_ext_t*pt;
	while(pt = opp_iterator_next(&iterator)) {
		struct chat_connection*chat = (struct chat_connection*)pt->obj_data;
		printf("Broadcasting to %s\n", aroop_txt_to_string(&chat->name));
		send(chat->fd, aroop_txt_to_string(msg), aroop_txt_length(msg), NULL);
	}
	opp_iterator_destroy(&iterator);
	return 0;
}

int broadcast_assign_to_room(struct chat_connection*chat, aroop_txt_t*room_name) {
	// find the chatroom
	struct internal_room*rm = NULL;
	opp_search(&room_factory, aroop_txt_get_hash(room_name), NULL, NULL, (void**)&rm);
	if(rm == NULL) {
		printf("Cannot find room %s\n", aroop_txt_to_string(room_name));
		return -1;
	}
	opp_list_add_noref(&rm->user_list, chat); // XXX we avoid circular reference ..
	chat->broadcast_data = rm; // we are not doing OPPREF because it will overflow our reference counter as more people join one group ..
	OPPUNREF(rm);
	chat->on_broadcast = broadcast_callback;
	return 0;
}

int broadcast_add_room(aroop_txt_t*room_name) {
	// XXX we have to load broadcast module before room module
	struct internal_room*rm = OPP_ALLOC1(&room_factory);
	aroop_txt_embeded_copy_on_demand(&rm->name, room_name);
	opp_set_hash(rm, aroop_txt_get_hash(&rm->name));
	return 0;
}

OPP_CB(internal_room) {
	struct internal_room*room = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			printf("creating user list\n");
			OPP_LIST_CREATE_NOLOCK(&room->user_list, 64);
		break;
		case OPPN_ACTION_FINALIZE:
			OPP_PFACTORY_DESTROY(&room->user_list);
			aroop_txt_destroy(&room->name);
		break;
	}
	return 0;
}

int broadcast_module_init() {
	NGINZ_SEARCHABLE_FACTORY_CREATE(&room_factory, 2, sizeof(struct internal_room), OPP_CB_FUNC(internal_room));
}
int broadcast_module_deinit() {
	OPP_PFACTORY_DESTROY(&room_factory);
}

C_CAPSULE_END

