
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include <aroop/aroop_memory_profiler.h>
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "db.h"
#include <libmemcached/memcached.h>

C_CAPSULE_START

static memcached_server_st *internal_servers = NULL;
static memcached_st *internal_memc = NULL;
int db_set(const char*key,const char*value) {
	if(internal_memc == NULL)
		return -1;
	memcached_return rc;
	rc= memcached_set(internal_memc, key, strlen(key), value, value?strlen(value):0, (time_t)0, (uint32_t)0);
	if(rc != MEMCACHED_SUCCESS)
		syslog(LOG_ERR, "Couldn't add server: %s\n",memcached_strerror(internal_memc, rc));
		
	return !(rc == MEMCACHED_SUCCESS);
}

int db_get(const char*key,aroop_txt_t*output) {
	if(internal_memc == NULL)
		return -1;
	size_t outlen = 0;
	uint32_t flags = 0;
	memcached_return rc;
	char*outval = memcached_get(internal_memc, key, strlen(key), &outlen, &flags, &rc);
	if(outval != NULL) {
		aroop_txt_embeded_buffer(output, outlen);
		aroop_txt_concat_string(output, outval);
		free(outval);
	}
	return !(rc == MEMCACHED_SUCCESS);
}

int db_get_int(const char*key) {
	int intval = -1;
	aroop_txt_t output = {};
	db_get(key, &output);
	if(!aroop_txt_is_empty(&output)) {
		aroop_txt_zero_terminate(&output);
		intval = aroop_txt_to_int(&output);
	}
	aroop_txt_destroy(&output);
	return intval;
}

int db_set_int(const char*key, int value) {
	aroop_txt_t valstr = {};
	aroop_txt_embeded_stackbuffer(&valstr, 32);
	aroop_txt_printf(&valstr, "%d", value);
	return db_set(key, aroop_txt_to_string(&valstr));
}

int db_module_init() {
	memcached_return rc;
	//memcached_server_st *memcached_servers_parse (char *server_strings);
	internal_memc= memcached_create(NULL);
	syslog(LOG_INFO, "creating memcached server\n");
	internal_servers= memcached_server_list_append(internal_servers, "localhost"/* server name */, 11211/* port */, &rc); 
	rc= memcached_server_push(internal_memc, internal_servers);
	if (rc == MEMCACHED_SUCCESS)
		syslog(LOG_INFO, "Added server successfully\n");
	else
		syslog(LOG_ERR, "Couldn't add server: %s\n",memcached_strerror(internal_memc, rc));
	return 0;
}

int db_module_deinit() {
	if(internal_memc != NULL)
		memcached_free(internal_memc);
	internal_memc = NULL;
	if(internal_servers != NULL)
		memcached_server_free(internal_servers);
	internal_servers = NULL;
}

C_CAPSULE_END
