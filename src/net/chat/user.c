

#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include "db.h"
#include "net/chat/user.h"

C_CAPSULE_START

static int build_name_key(aroop_txt_t*name, aroop_txt_t*output) {
	aroop_txt_concat_string(output, "user/");
	aroop_txt_concat(output, name);
	aroop_txt_zero_terminate(output);
	return 0;
}

int try_login(aroop_txt_t*name) {
	int ret = 0;
	if(aroop_txt_is_empty_magical(name)) // sanity check
		return -1;
	aroop_txt_t name_key = {};
	aroop_txt_embeded_stackbuffer(&name_key, 128);
	build_name_key(name, &name_key);
	aroop_txt_t result = {};
	db_get(aroop_txt_to_string(&name_key), &result);
	if(!aroop_txt_is_empty(&result)) {
		printf(" user value %s\n", aroop_txt_to_string(&result));
		ret = -1;
	} else {
		aroop_txt_embeded_rebuild_and_set_static_string(&result, "1");
		aroop_txt_zero_terminate(&result);
		db_set(aroop_txt_to_string(&name_key), aroop_txt_to_string(&result));
	}
	aroop_txt_destroy(&result);
	return ret;
}

int logoff_user(aroop_txt_t*name) {
	int ret = 0;
	if(aroop_txt_is_empty_magical(name)) // sanity check
		return -1;
	aroop_txt_t name_key = {};
	aroop_txt_embeded_stackbuffer(&name_key, 128);
	build_name_key(name, &name_key);
	aroop_txt_t result = {};
	aroop_memclean_raw2(&result);
	db_set(aroop_txt_to_string(&name_key), aroop_txt_to_string(&result));
	aroop_txt_destroy(&result);
	return ret;

}

C_CAPSULE_END

