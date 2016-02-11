
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include <aroop/aroop_memory_profiler.h>
#include "plugin.h"
#include "plugin_manager.h"
#include "db.h"
#include <libmemcached/memcached.h>

C_CAPSULE_START

static memcached_server_st *internal_servers = NULL;
static memcached_st *internal_memc = NULL;
int db_save(char*key,char*value) {
	if(internal_memc == NULL)
		return -1;
	memcached_return rc;
	printf("We are saving %s\n", key);
	rc= memcached_set(internal_memc, key, strlen(key), value, strlen(value), (time_t)0, (uint32_t)0);
	if(rc == MEMCACHED_SUCCESS)
		printf("We saved the key successfully\n");
	else
		printf("Couldn't add server: %s\n",memcached_strerror(internal_memc, rc));
		
	return !(rc == MEMCACHED_SUCCESS);
}

int db_get(char*key,aroop_txt_t*output) {
	if(internal_memc == NULL)
		return -1;
	size_t outlen = 0;
	uint32_t flags = 0;
	memcached_return rc;
	char*outval = memcached_get(internal_memc, key, strlen(key), &outlen, &flags, &rc);
	if(outval != NULL) {
		aroop_txt_embeded_buffer(output, outlen);
		aroop_txt_embeded_copy_string(output, outval);
		free(outval);
	}
	return !(rc == MEMCACHED_SUCCESS);
}

int db_module_init() {
	memcached_return rc;
	//memcached_server_st *memcached_servers_parse (char *server_strings);
	internal_memc= memcached_create(NULL);
	printf("creating memcached server\n");
	internal_servers= memcached_server_list_append(internal_servers, "localhost"/* server name */, 11211/* port */, &rc); 
	rc= memcached_server_push(internal_memc, internal_servers);
	if (rc == MEMCACHED_SUCCESS)
		printf("Added server successfully\n");
	else
		fprintf(stderr,"Couldn't add server: %s\n",memcached_strerror(internal_memc, rc));
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
