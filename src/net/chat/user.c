

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

#define USER_PREFIX "user/"
static int build_name_key(aroop_txt_t*name, aroop_txt_t*output) {
	aroop_txt_concat_string(output, USER_PREFIX);
	aroop_txt_concat(output, name);
	aroop_txt_zero_terminate(output);
	return 0;
}

static int (*on_login_callback)(int token, aroop_txt_t*name, int success);
#if 0
int try_login(aroop_txt_t*name) {
	int ret = 0;
	if(aroop_txt_is_empty_magical(name)) // sanity check
		return -1;
	aroop_txt_t name_key = {};
	aroop_txt_embeded_stackbuffer(&name_key, 128);
	build_name_key(name, &name_key);
	aroop_txt_t result = {};
	db_get(aroop_txt_to_string(&name_key), &result); // needs cleanup
	if(!aroop_txt_is_empty(&result)) {
		//syslog(LOG_INFO, "User already exists %s\n", aroop_txt_to_string(&result));
		ret = -1;
	} else {
		// XXX there is a race condition ..
		aroop_txt_destroy(&result);
		aroop_txt_embeded_rebuild_and_set_static_string(&result, "1");
		aroop_txt_zero_terminate(&result);
		db_set(aroop_txt_to_string(&name_key), aroop_txt_to_string(&result));
	}
	aroop_txt_destroy(&result); // cleanup
	return ret;
}
#else
int async_try_login(aroop_txt_t*name, int on_try_login_complete(int token, aroop_txt_t*name, int success), int token) {
	int ret = 0;
	if(aroop_txt_is_empty_magical(name)) // sanity check
		return -1;
	aroop_txt_t name_key = {};
	aroop_txt_embeded_stackbuffer(&name_key, 128);
	build_name_key(name, &name_key);
	aroop_txt_t value = {};
	aroop_txt_embeded_rebuild_and_set_static_string(&value, "1");
	aroop_txt_t login_hook = {};
	aroop_txt_embeded_set_static_string(&login_hook, "chatuser/on/login");
	async_db_compare_and_swap(token, &login_hook, name, &value, NULL);
	on_login_callback = on_try_login_complete;
	return 0;
}
#endif


int logoff_user(struct chat_connection*chat) {
	int ret = 0;
	if(aroop_txt_is_empty_magical(&chat->name)) // sanity check
		return -1;
	aroop_txt_t name_key = {};
	aroop_txt_embeded_stackbuffer(&name_key, 128);
	build_name_key(&chat->name, &name_key);
	aroop_txt_t result = {};
	aroop_memclean_raw2(&result);
	db_set(aroop_txt_to_string(&name_key), aroop_txt_to_string(&result));
	aroop_txt_destroy(&result);
	chat->state &= ~CHAT_LOGGED_IN;
	return ret;

}

static int user_try_login_response_hook(aroop_txt_t*bin, aroop_txt_t*output) {
	aroop_assert(!aroop_txt_is_empty_magical(bin));
	// 0 = pid, 1 = srcpid, 2 = command, 3 = token, 4 = cb_hook, 5 = success, 6 = key, 7 = newvalue
	int cb_token = 0;
	int success = 0;
	aroop_txt_t name = {};
	binary_unpack_int(bin, 3, &cb_token); // id/token
	binary_unpack_int(bin, 5, &success);
	binary_unpack_string(bin, 6, &name);
	if(!aroop_txt_is_empty(&name)) {
		aroop_txt_shift(&name, sizeof(USER_PREFIX));
	}
	on_login_callback(cb_token, &name, success);
	aroop_txt_destroy(&name);
	return 0;
}

static int user_async_hook_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "chat user", "hookup", plugin_space, __FILE__, "It helps chat user to collect the hooks.\n");
}

int user_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chatuser/on/login");
	pm_plug_callback(&plugin_space, user_try_login_response_hook , user_async_hook_desc);
}

int user_module_deinit() {
	pm_unplug_callback(0, user_try_login_response_hook);
}

C_CAPSULE_END

