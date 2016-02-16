
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_iterator.h>
#include <aroop/opp/opp_list.h>
#include "nginz_config.h"
#include "plugin.h"
#include "log.h"
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

static int broadcast_callback_helper(struct chat_connection*chat, struct internal_room*rm, aroop_txt_t*msg) {
	// iterate through all
	struct opp_iterator iterator = {};
	opp_iterator_create(&iterator, &rm->user_list, OPPN_ALL, 0, 0);
	opp_pointer_ext_t*pt;
	while(pt = opp_iterator_next(&iterator)) {
		struct chat_connection*other = (struct chat_connection*)pt->obj_data;
		if(chat == other)
			continue; // do not broadcast to self

		// broadcast message to others
		send(other->fd, aroop_txt_to_string(msg), aroop_txt_length(msg), 0); // send it to other
	}
	opp_iterator_destroy(&iterator);
	return 0;
}
static int broadcast_callback(struct chat_connection*chat, aroop_txt_t*msg) {
	struct internal_room*rm = (struct internal_room*)(chat->broadcast_data);
	if(!rm) {
		syslog(LOG_ERR, "We do not know how we can broadcast it\n");
		return -1;
	}
	// iterate all the user
	aroop_txt_t resp = {};
	aroop_txt_embeded_stackbuffer(&resp, 1024);
	aroop_txt_concat(&resp, &chat->name); // show the message sender name
	aroop_txt_concat_char(&resp, ':');
	aroop_txt_concat(&resp, msg); // show the message
	return broadcast_callback_helper(chat, rm, &resp);
}

static int broadcast_room_greet(struct chat_connection*chat, struct internal_room*rm) {
	aroop_txt_t resp = {};
	aroop_txt_embeded_stackbuffer(&resp, 1024);
	aroop_txt_concat_string(&resp, "Entering room:");
	aroop_txt_concat(&resp, &rm->name);
	aroop_txt_concat_char(&resp, '\n');
	send(chat->fd, aroop_txt_to_string(&resp), aroop_txt_length(&resp), 0);

	// now print the user list ..
	struct opp_iterator iterator = {};
	opp_iterator_create(&iterator, &rm->user_list, OPPN_ALL, 0, 0);
	opp_pointer_ext_t*pt;
	while(pt = opp_iterator_next(&iterator)) {
		struct chat_connection*other = (struct chat_connection*)pt->obj_data;
		aroop_txt_set_length(&resp, 0);
		aroop_txt_concat_char(&resp, '\t');
		aroop_txt_concat_char(&resp, '*');
		aroop_txt_concat_char(&resp, ' ');
		aroop_txt_concat(&resp, &other->name);
		if(chat == other) {
			// say it is you 
			aroop_txt_concat_string(&resp, "(** this is you)");
		}
		aroop_txt_concat_char(&resp, '\n');
		send(chat->fd, aroop_txt_to_string(&resp), aroop_txt_length(&resp), 0);
	}
	opp_iterator_destroy(&iterator);

	// end
	aroop_txt_set_length(&resp, 0);
	aroop_txt_concat_string(&resp, "end of list\n");
	send(chat->fd, aroop_txt_to_string(&resp), aroop_txt_length(&resp), 0);

	// broadcast that there is new user
	aroop_txt_set_length(&resp, 0);
	aroop_txt_concat_string(&resp, "\t* user joined chat:");
	aroop_txt_concat(&resp, &chat->name);
	aroop_txt_concat_char(&resp, '\n');
	broadcast_callback_helper(chat, rm, &resp);
}

static int broadcast_room_bye(struct chat_connection*chat, struct internal_room*rm) {
	aroop_txt_t resp = {};
	aroop_txt_embeded_stackbuffer(&resp, 1024);
	aroop_txt_concat_string(&resp, "\t* user has left chat:");
	aroop_txt_concat(&resp, &chat->name);
	aroop_txt_concat_char(&resp, '\n');
	broadcast_callback_helper(NULL/* do not ommit the user */, rm, &resp);
}

int broadcast_room_join(struct chat_connection*chat, aroop_txt_t*room_name) {
	// find the chatroom
	struct internal_room*rm = NULL;
	opp_search(&room_factory, aroop_txt_get_hash(room_name), NULL, NULL, (void**)&rm);
	if(rm == NULL) {
		syslog(LOG_ERR, "Cannot find room %s\n", aroop_txt_to_string(room_name));
		return -1;
	}
	do {
		if(chat->broadcast_data == rm) {
			// user already joined the room
			break;
		}
		if(chat->broadcast_data) {
			// leave the old room
			broadcast_room_leave(chat);
		}
		aroop_assert(chat->broadcast_data == NULL);
		opp_list_add_noref(&rm->user_list, chat);
		OPPREF(chat);
		// XXX we avoid circular reference ..
		chat->broadcast_data = rm; // we are not doing OPPREF because it will overflow our reference counter as more people join one group ..
		chat->on_broadcast = broadcast_callback;
	} while(0);
	OPPUNREF(rm);
	// show room information 
	broadcast_room_greet(chat, (struct internal_room*)chat->broadcast_data);
	chat_room_set_user_count(room_name, OPP_FACTORY_USE_COUNT(&((struct internal_room*)chat->broadcast_data)->user_list));
	return 0;
}

int broadcast_room_leave(struct chat_connection*chat) {
	// find the chatroom
	struct internal_room*rm = (struct internal_room*)chat->broadcast_data;
	if(!rm)
		return 0;
	broadcast_room_bye(chat, rm);

	// prune the user from the list
	struct opp_iterator iterator = {};
	opp_iterator_create(&iterator, &rm->user_list, OPPN_ALL, 0, 0);
	opp_pointer_ext_t*pt = NULL;
	while(pt = opp_iterator_next(&iterator)) {
		struct chat_connection*other = (struct chat_connection*)pt->obj_data;
		if(chat == other) {
			OPPUNREF(pt);
			break;
		}
	}
	opp_iterator_destroy(&iterator);

	chat->on_broadcast = NULL;
	chat->broadcast_data = NULL;
	chat_room_set_user_count(&rm->name, OPP_FACTORY_USE_COUNT(&rm->user_list));
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
			//printf("creating user list\n");
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

