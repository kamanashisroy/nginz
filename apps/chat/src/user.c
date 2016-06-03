

#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include "db.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "log.h"
#include "streamio.h"
#include "chat.h"
#include "chat/user.h"
#include "async_db.h"

C_CAPSULE_START

static int build_name_key(aroop_txt_t*name, aroop_txt_t*output) {
	aroop_txt_concat_string(output, USER_PREFIX);
	aroop_txt_concat(output, name);
	aroop_txt_zero_terminate(output);
	return 0;
}

int async_try_login(aroop_txt_t*name, int token, aroop_txt_t*response_hook) {
	if(aroop_txt_is_empty_magical(name)) // sanity check
		return -1;
	aroop_txt_t name_key = {};
	aroop_txt_embeded_stackbuffer(&name_key, 128);
	build_name_key(name, &name_key);
	//syslog(LOG_NOTICE, "requesting user %s\n", aroop_txt_to_string(name));
	async_db_compare_and_swap(token, response_hook, &name_key, name, NULL);
	return 0;
}


int logoff_user(struct chat_connection*chat) {
	if(aroop_txt_is_empty_magical(&chat->name)) // sanity check
		return -1;
	aroop_txt_t name_key = {};
	aroop_txt_embeded_stackbuffer(&name_key, 128);
	build_name_key(&chat->name, &name_key);
	async_db_unset(-1, NULL, &name_key);
	chat->state &= ~CHAT_LOGGED_IN;
	return 0;
}


int user_module_init() {
	return 0;
}

int user_module_deinit() {
	return 0;
}

C_CAPSULE_END

