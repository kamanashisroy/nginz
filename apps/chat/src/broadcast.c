
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_iterator.h>
#include <aroop/opp/opp_list.h>
#include "nginz_config.h"
#include "sys/socket.h"
#include "plugin.h"
#include "log.h"
#include "streamio.h"
#include "chat.h"
#include "chat/chat_plugin_manager.h"
#include "chat/broadcast.h"

C_CAPSULE_START

struct internal_room {
	struct opp_object_ext _ext;
	aroop_txt_t name;
	struct opp_factory user_list;
};
static struct opp_factory room_factory;

int broadcast_private_message(struct chat_connection*chat, aroop_txt_t*to, aroop_txt_t*msg) {
	struct internal_room*rm = (struct internal_room*)(chat->callback_data);
	if(!rm) {
		syslog(LOG_ERR, "We do not know how we send private message\n");
		return 0;
	}
	// iterate through all
	struct opp_iterator iterator = {};
	opp_iterator_create(&iterator, &rm->user_list, OPPN_ALL, 0, 0);
	opp_pointer_ext_t*pt;
	while(pt = opp_iterator_next(&iterator)) {
		struct chat_connection*other = (struct chat_connection*)pt->obj_data;
		if(!aroop_txt_equals(to, &other->name))
			continue; // do not broadcast to self
		// pm
		other->strm.send(&other->strm, msg, 0);
		break;
	}
	opp_iterator_destroy(&iterator);
	return 0;
}

static int broadcast_callback_helper(struct chat_connection*chat, struct internal_room*rm, aroop_txt_t*msg) {
	// iterate through all
	struct opp_iterator iterator = {};
	aroop_assert(rm);
	opp_iterator_create(&iterator, &rm->user_list, OPPN_ALL, 0, 0);
	opp_pointer_ext_t*pt;
	while(pt = opp_iterator_next(&iterator)) {
		struct chat_connection*other = (struct chat_connection*)pt->obj_data;
		if(chat == other)
			continue; // do not broadcast to self

		// broadcast message to others
		other->strm.send(&other->strm, msg, 0);
	}
	opp_iterator_destroy(&iterator);
	return 0;
}
static int broadcast_callback(struct chat_connection*chat, aroop_txt_t*msg) {
	struct internal_room*rm = (struct internal_room*)(chat->callback_data);
	if(!rm) {
		syslog(LOG_ERR, "We do not know how we can broadcast it\n");
		return -1;
	}
	// iterate all the user
	aroop_txt_t resp = {};
	aroop_txt_embeded_stackbuffer(&resp, NGINZ_MAX_CHAT_USER_NAME_SIZE + NGINZ_MAX_CHAT_MSG_SIZE);
	aroop_txt_concat(&resp, &chat->name); // show the message sender name
	aroop_txt_concat_char(&resp, ':');
	aroop_txt_concat(&resp, msg); // show the message
	// chat->strm.send(&chat->strm, msg, 0); // show the message to the user himself ..
	return broadcast_callback_helper(chat, rm, &resp);
}

static int broadcast_room_greet(struct chat_connection*chat, struct internal_room*rm) {
	aroop_txt_t resp = {};
	aroop_txt_embeded_stackbuffer(&resp, 1024);
	aroop_txt_concat_string(&resp, "Entering room:");
	aroop_txt_concat(&resp, &rm->name);
	aroop_txt_concat_char(&resp, '\n');
	chat->strm.send(&chat->strm, &resp, MSG_MORE);

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
		chat->strm.send(&chat->strm, &resp, MSG_MORE);
	}
	opp_iterator_destroy(&iterator);

	// end
	aroop_txt_set_length(&resp, 0);
	aroop_txt_concat_string(&resp, "end of list\n");
	chat->strm.send(&chat->strm, &resp, 0);

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

static int broadcast_room_send_join_failed(struct chat_connection*chat) {
	aroop_txt_t resp = {};
	aroop_txt_embeded_set_static_string(&resp, "Sorry, room is full\n");
	chat->strm.send(&chat->strm, &resp, 0);
	return 0;
}

int broadcast_room_join(struct chat_connection*chat, aroop_txt_t*room_name) {
	/* sanity check */
	if(!(chat->state & CHAT_LOGGED_IN)) {
		syslog(LOG_ERR, "The user tries to join a room while he is not logged in\n");
		return -1;
	}
	// find the chatroom
	struct internal_room*rm = NULL;
	opp_search(&room_factory, aroop_txt_get_hash(room_name), NULL, NULL, (void**)&rm);
	if(rm == NULL) {
		syslog(LOG_ERR, "Cannot find room %s\n", aroop_txt_to_string(room_name));
		return -1;
	}
	if(OPP_FACTORY_USE_COUNT(&rm->user_list) >= NGINZ_MAX_CHAT_ROOM_USER) {
		broadcast_room_send_join_failed(chat);
		return -1;
	}
	do {
		if(chat->callback_data == rm) {
			aroop_assert(chat->state & CHAT_IN_ROOM);
			// user already joined the room
			break;
		}
		if(chat->callback_data) {
			aroop_assert(chat->state & CHAT_IN_ROOM);
			// leave the old room
			broadcast_room_leave(chat);
		}
		aroop_assert(chat->callback_data == NULL);
		opp_list_add_noref(&rm->user_list, chat);
		OPPREF(chat);
		// XXX we avoid circular reference ..
		chat->callback_data = rm; // we are not doing OPPREF because it will overflow our reference counter as more people join one group ..
		chat->on_response_callback = broadcast_callback;
		chat->state |= CHAT_IN_ROOM;
	} while(0);
	aroop_assert(chat->callback_data != NULL);
	OPPUNREF(rm);
	rm = (struct internal_room*)chat->callback_data;
	// show room information 
	broadcast_room_greet(chat, rm);
	chat_room_set_user_count(room_name, OPP_FACTORY_USE_COUNT(&rm->user_list));
	return 0;
}

int broadcast_room_leave(struct chat_connection*chat) {
	// find the chatroom
	struct internal_room*rm = (struct internal_room*)chat->callback_data;
	if(!rm)
		return 0;
	aroop_assert(chat->state & CHAT_IN_ROOM);
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

	chat->callback_data = NULL;
	chat->on_response_callback = NULL;
	chat->state &= (~CHAT_IN_ROOM);
	chat_room_set_user_count(&rm->name, OPP_FACTORY_USE_COUNT(&rm->user_list));
	return 0;
}

int broadcast_add_room(aroop_txt_t*room_name) {
	// XXX we have to load broadcast module before room module
	struct internal_room*rm = OPP_ALLOC1(&room_factory);
	aroop_txt_embeded_copy_deep(&rm->name, room_name);
	opp_set_hash(rm, aroop_txt_get_hash(&rm->name)); // set the hash so that it can be searched easily
	return 0;
}

OPP_CB(internal_room) {
	struct internal_room*room = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			aroop_memclean_raw2(&room->name);
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

