

#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include "db.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "log.h"
#include "net/streamio.h"
#include "net/chat.h"
#include "net/chat/user.h"
#include "parallel/async_db.h"

C_CAPSULE_START

static int build_name_key(aroop_txt_t*name, aroop_txt_t*output) {
	aroop_txt_concat_string(output, USER_PREFIX);
	aroop_txt_concat(output, name);
	aroop_txt_zero_terminate(output);
	return 0;
}

int async_try_login(aroop_txt_t*name, int token, aroop_txt_t*response_hook) {
	int ret = 0;
	if(aroop_txt_is_empty_magical(name)) // sanity check
		return -1;
	aroop_txt_t name_key = {};
	aroop_txt_embeded_stackbuffer(&name_key, 128);
	build_name_key(name, &name_key);
	syslog(LOG_NOTICE, "requesting user %s\n", aroop_txt_to_string(name));
	async_db_compare_and_swap(token, response_hook, &name_key, name, NULL);
	return 0;
}


int logoff_user(struct chat_connection*chat) {
	int ret = 0;
	if(aroop_txt_is_empty_magical(&chat->name)) // sanity check
		return -1;
	aroop_txt_t name_key = {};
	aroop_txt_embeded_stackbuffer(&name_key, 128);
	build_name_key(&chat->name, &name_key);
	async_db_unset(-1, NULL, &name_key);
	chat->state &= ~CHAT_LOGGED_IN;
	return 0;
#if 0
	aroop_txt_t result = {};
	aroop_memclean_raw2(&result);
	db_set(aroop_txt_to_string(&name_key), aroop_txt_to_string(&result));
	aroop_txt_destroy(&result);
	chat->state &= ~CHAT_LOGGED_IN;
	return ret;
#else
#endif
}


int user_module_init() {
}

int user_module_deinit() {
}

C_CAPSULE_END

