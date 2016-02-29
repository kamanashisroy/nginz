
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_hash_table.h>
#include "nginz_config.h"
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "event_loop.h"
#include "parallel/pipeline.h"

C_CAPSULE_START

static int masterpid = 0;
int async_db_compare_and_swap(int cb_token, aroop_txt_t*cb_hook, aroop_txt_t*key, aroop_txt_t*newval, aroop_txt_t*oldval) {
	aroop_assert(!aroop_txt_is_empty_magical(cb_hook));
	aroop_assert(!aroop_txt_is_empty_magical(key));
	if(oldval)
		aroop_assert(!aroop_txt_is_empty_magical(newval));
	aroop_txt_t app = {};
	if(oldval) {
		aroop_txt_embeded_set_static_string(&app, "asyncdb/cas/request"); 
	} else {
		if(newval)
			aroop_txt_embeded_set_static_string(&app, "asyncdb/sin/request"); 
		else
			aroop_txt_embeded_set_static_string(&app, "asyncdb/unset/request"); 
	}
	aroop_txt_t bin = {}; // binary response
	// send response
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset(&bin);
	// 0 = pid, 1 = srcpid, 2 = command, 3 = token, 4 = cb_hook, 5 = key, 6 = newval, 7 = oldval
	binary_pack_int(&bin, masterpid); // send destination pid
	binary_pack_int(&bin, getpid()); // send source pid
	binary_pack_string(&bin, &app);
	binary_pack_int(&bin, cb_token); // id/token
	binary_pack_string(&bin, cb_hook); // callback hook
	binary_pack_string(&bin, key);
	if(newval)
		binary_pack_string(&bin, newval);
	if(oldval)
		binary_pack_string(&bin, oldval);
	//syslog(LOG_NOTICE, "compare and swap .. sending to master %d", masterpid);
	pp_bubble_up(&bin);
	return 0;
}

int async_db_unset(int cb_token, aroop_txt_t*cb_hook, aroop_txt_t*key) {
	return async_db_compare_and_swap(cb_token, cb_hook, key, NULL, NULL);
}

static opp_hash_table_t global_db; // TODO create partitioned db in future

static int async_db_op_reply(int cb_token, aroop_txt_t*cb_hook, int destpid, aroop_txt_t*app, int success, aroop_txt_t*key, aroop_txt_t*newval) {
	aroop_assert(!aroop_txt_is_empty_magical(cb_hook));
	aroop_assert(!aroop_txt_is_empty_magical(app));
	aroop_txt_t bin = {}; // binary response
	// send response
	// 0 = pid, 1 = src pid, 2 = command, 3 = token, 4 = cb_hook, 5 = success
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset(&bin);
	binary_pack_int(&bin, destpid); // send destination pid
	binary_pack_int(&bin, getpid()); // send destination pid
	binary_pack_string(&bin, app);
	binary_pack_int(&bin, cb_token); // id/token
	binary_pack_string(&bin, cb_hook); // id/token
	binary_pack_int(&bin, success); // means success
	binary_pack_string(&bin, key);
	if(newval != NULL)
		binary_pack_string(&bin, newval);
	//syslog(LOG_NOTICE, "replying .. sending to dest %d:%s", destpid, aroop_txt_to_string(app));
	pp_bubble_down(&bin);
	return 0;
}

static int async_db_op_helper(aroop_txt_t*key, aroop_txt_t*newval, aroop_txt_t*expval) {
	if(aroop_txt_is_empty(key)) { // we cannot process the request ..
		return -1;
	}
	if(newval == NULL) {
		opp_hash_table_set(&global_db, key, NULL); // unset
		return 0;
	}
	aroop_txt_t*oldval = (aroop_txt_t*)opp_hash_table_get(&global_db, key);
	if(!((oldval == NULL && expval == NULL) || (oldval != NULL && expval != NULL && aroop_txt_equals(expval, oldval)))) {
		if(oldval)OPPUNREF(oldval);
		return -1;
	}
	aroop_txt_t*xnval = aroop_txt_new_copy_deep(newval, NULL);
	aroop_txt_t*xkey = aroop_txt_new_copy_deep(key, NULL);
	//syslog(LOG_NOTICE, "set .. for %s = %s", aroop_txt_to_string(key), aroop_txt_to_string(newval));
	opp_hash_table_set(&global_db, xkey, xnval);
	OPPUNREF(xnval);
	OPPUNREF(xkey);
	if(oldval)OPPUNREF(oldval);
	return 0;
}

static int async_db_CAS_hook(aroop_txt_t*bin, aroop_txt_t*output) {
	/**
	 * The database is available only in the master process
	 */
	aroop_assert(is_master());
	aroop_txt_t key = {}; // the key to set
	aroop_txt_t expval = {}; // the val to compare with the oldval
	aroop_txt_t newval = {}; // the val to set
	int destpid = 0;
	int srcpid = 0;
	int cb_token = 0;
	aroop_txt_t cb_hook = {};
	// 0 = pid, 1 = srcpid, 2 = command, 3 = token, 4 = cb_hook, 5 = key, 6 = newval, 7 = oldval
	binary_unpack_int(bin, 0, &destpid);
	binary_unpack_int(bin, 1, &srcpid);
	binary_unpack_int(bin, 3, &cb_token);
	binary_unpack_string(bin, 4, &cb_hook); // needs cleanup
	binary_unpack_string(bin, 5, &key); // needs cleanup
	binary_unpack_string(bin, 6, &newval); // needs cleanup
	binary_unpack_string(bin, 7, &expval); // needs cleanup
	int success = 0;
	success = !async_db_op_helper(&key, &newval, &expval);
	if(destpid > 0) {
		aroop_txt_t app = {};
		aroop_txt_embeded_set_static_string(&app, "asyncdb/response"); 
		async_db_op_reply(cb_token, &cb_hook, srcpid, &app, success, &key, &newval);
	}
	// cleanup
	aroop_txt_destroy(&key);
	aroop_txt_destroy(&newval);
	aroop_txt_destroy(&expval);
	aroop_txt_destroy(&cb_hook);
	return 0;
}

static int async_db_set_if_null_hook(aroop_txt_t*bin, aroop_txt_t*output) {
	/**
	 * The database is available only in the master process
	 */
	aroop_assert(is_master());
	aroop_txt_t key = {}; // the key to set
	aroop_txt_t newval = {}; // the val to set
	int destpid = 0;
	int srcpid = 0;
	int cb_token = 0;
	aroop_txt_t cb_hook = {};
	// 0 = pid, 1 = srcpid, 2 = command, 3 = token, 4 = cb_hook, 5 = key, 6 = newval, 7 = oldval
	binary_unpack_int(bin, 0, &destpid);
	binary_unpack_int(bin, 1, &srcpid);
	binary_unpack_int(bin, 3, &cb_token);
	binary_unpack_string(bin, 4, &cb_hook); // needs cleanup
	binary_unpack_string(bin, 5, &key); // needs cleanup
	binary_unpack_string(bin, 6, &newval); // needs cleanup
	int success = 0;
	success = !async_db_op_helper(&key, &newval, NULL);
	if(destpid > 0) {
		aroop_txt_t app = {};
		aroop_txt_embeded_set_static_string(&app, "asyncdb/response"); 
		async_db_op_reply(cb_token, &cb_hook, srcpid, &app, success, &key, &newval);
	}
	// cleanup
	aroop_txt_destroy(&cb_hook);
	aroop_txt_destroy(&key);
	aroop_txt_destroy(&newval);
	return 0;
}

static int async_db_unset_hook(aroop_txt_t*bin, aroop_txt_t*output) {
	/**
	 * The database is available only in the master process
	 */
	aroop_assert(is_master());
	aroop_txt_t key = {}; // the key to set
	int destpid = 0;
	int srcpid = 0;
	int cb_token = 0;
	aroop_txt_t cb_hook = {};
	// 0 = pid, 1 = srcpid, 2 = command, 3 = token, 4 = cb_hook, 5 = key, 6 = newval, 7 = oldval
	binary_unpack_int(bin, 0, &destpid);
	binary_unpack_int(bin, 1, &srcpid);
	binary_unpack_int(bin, 3, &cb_token);
	binary_unpack_string(bin, 4, &cb_hook); // needs cleanup
	binary_unpack_string(bin, 5, &key); // needs cleanup
	syslog(LOG_NOTICE, "[pid:%d]\tdeleting:%s", getpid(), aroop_txt_to_string(&key));
	int success = 0;
	success = !async_db_op_helper(&key, NULL, NULL);
	if(destpid > 0) {
		aroop_txt_t app = {};
		aroop_txt_embeded_set_static_string(&app, "asyncdb/response"); 
		async_db_op_reply(cb_token, &cb_hook, srcpid, &app, success, &key, NULL);
	}
	// cleanup
	aroop_txt_destroy(&cb_hook);
	aroop_txt_destroy(&key);
	return 0;
}

static int async_db_response_hook(aroop_txt_t*bin, aroop_txt_t*output) {
	aroop_assert(!aroop_txt_is_empty_magical(bin));
	// 0 = pid, 1 = srcpid, 2 = command, 3 = token, 4 = cb_hook, 5 = success
	aroop_txt_t cb_hook = {};
	binary_unpack_string(bin, 4, &cb_hook); // needs cleanup
	//syslog(LOG_NOTICE, "[pid:%d]\texecuting command:%s", getpid(), aroop_txt_to_string(&cb_hook));
	aroop_txt_t outval = {};
	pm_call(&cb_hook, bin, &outval);
	aroop_txt_destroy(&cb_hook);
	aroop_txt_destroy(&outval);
	return 0;
}

static int async_db_hook_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "async db", "asyncdb", plugin_space, __FILE__, "It helps cas and other rpc db values in master process.\n");
}

static int async_db_dump_helper(void*cb_data, void*key, void*value) {
	aroop_txt_t*output = (aroop_txt_t*)cb_data;
	aroop_txt_t*xkey = (aroop_txt_t*)key;
	aroop_txt_t*xval = (aroop_txt_t*)value;
	aroop_txt_concat_char(output, '[');
	aroop_txt_concat(output, xkey);
	aroop_txt_concat_char(output, ']');
	aroop_txt_concat_char(output, '>');
	aroop_txt_concat_char(output, '[');
	aroop_txt_concat(output, xval);
	aroop_txt_concat_char(output, ']');
	aroop_txt_concat_char(output, '\n');
	return 0;
}

static int async_db_dump_hook(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_embeded_buffer(output, 255);
	opp_hash_table_traverse(&global_db, async_db_dump_helper, output, OPPN_ALL, 0, 0);
	return 0;
}

static int async_db_dump_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "dbdump", "shake", plugin_space, __FILE__, "It dumps the db entries from database.\n");
}

int async_db_init() {
	masterpid = getpid();
	if(is_master())
		opp_hash_table_create(&global_db, 16, 0, aroop_txt_get_hash_cb, aroop_txt_equals_cb);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "asyncdb/cas/request");
	pm_plug_callback(&plugin_space, async_db_CAS_hook, async_db_hook_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "asyncdb/sin/request");
	pm_plug_callback(&plugin_space, async_db_set_if_null_hook, async_db_hook_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "asyncdb/unset/request");
	pm_plug_callback(&plugin_space, async_db_unset_hook, async_db_hook_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "asyncdb/response");
	pm_plug_callback(&plugin_space, async_db_response_hook , async_db_hook_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/dbdump");
	pm_plug_callback(&plugin_space, async_db_dump_hook, async_db_dump_desc);
	return 0;
}

int async_db_deinit() {
	if(is_master())
		opp_hash_table_destroy(&global_db);
	pm_unplug_callback(0, async_db_CAS_hook);
	pm_unplug_callback(0, async_db_set_if_null_hook);
	pm_unplug_callback(0, async_db_unset_hook);
	pm_unplug_callback(0, async_db_response_hook);
	pm_unplug_callback(0, async_db_dump_hook);
}

C_CAPSULE_END

