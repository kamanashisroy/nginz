
#include "aroop/aroop_core.h"
#include "aroop/core/thread.h"
#include "aroop/core/xtring.h"
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "aroop/opp/opp_any_obj.h"
#include "aroop/opp/opp_str2.h"
#include "aroop/aroop_memory_profiler.h"
#include "plugin.h"

C_CAPSULE_START

enum plugin_category {
	CALLBACK_PLUGIN = 0,
	COMPOSITE_PLUGIN,
	BRIDGE_PLUGIN,
};

struct composite_plugin { 
	struct opp_factory factory;
	opp_hash_table_t table;
};


struct internal_plugin {
	int category;
	union {
		int (*callback)(aroop_txt_t*input, aroop_txt_t*output);
		int (*bridge)(int signature, void*x);
		struct composite_plugin*inner;
	} x;
};

static struct opp_factory cplugs;
struct composite_plugin*composite_plugin_create() {
	return OPP_ALLOC1(&cplugs);
}

OPP_CB(internal_plugin) {
	struct internal_plugin*plugin = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			plugin->category = 0;
		break;
		case OPPN_ACTION_FINALIZE:
		break;
	}
	return 0;
}

OPP_CB(composite_plugin) {
	struct composite_plugin*cplug = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			OPP_PFACTORY_CREATE(&cplug->factory, 2, sizeof(struct internal_plugin), OPP_CB_FUNC(internal_plugin));
			opp_hash_table_create(&cplug->table, 2, 0, aroop_txt_get_hash_cb, aroop_txt_equals_cb);
		break;
		case OPPN_ACTION_FINALIZE:
			OPP_PFACTORY_DESTROY(&cplug->factory);
			opp_hash_table_destroy(&cplug->table);
			
		break;
	}
	return 0;
}

int plugin_init() {
	OPP_PFACTORY_CREATE(&cplugs, 4, sizeof(struct composite_plugin), OPP_CB_FUNC(composite_plugin));
}

int plugin_deinit() {
	OPP_PFACTORY_DESTROY(&cplugs);
}

static int composite_plug_helper(struct composite_plugin*container
	, aroop_txt_t*plugin_space
	, int (*callback)(aroop_txt_t*input, aroop_txt_t*output)
	, int (*bridge)(int signature, void*x)
	, struct composite_plugin*inner) {
	struct internal_plugin*plugin = OPP_ALLOC1(&container->factory);
	aroop_assert(plugin != NULL);
	if(callback != NULL) {
		plugin->category = CALLBACK_PLUGIN;
		plugin->x.callback = callback;
	} else if(bridge != NULL) {
		plugin->category = BRIDGE_PLUGIN;
		plugin->x.bridge = bridge;
	} else if(inner != NULL){
		plugin->category = COMPOSITE_PLUGIN;
		plugin->x.inner = inner;
	} else {
		aroop_assert(!"Unrecognised plugin\n");
	}
	aroop_txt_t*space = aroop_txt_new_copy_deep(plugin_space, NULL);
	aroop_assert(space != NULL);
	opp_hash_table_set(&(container->table), space, plugin);
	int ret = 0; // XXX TOKEN DOES NOT WORK
	OPPUNREF(plugin); // cleanup : unref the plugin, it is already saved in the hashtable
	return ret;
}


int composite_plug_callback(struct composite_plugin*container, aroop_txt_t*plugin_space, int (*callback)(aroop_txt_t*input, aroop_txt_t*output)) {
	composite_plug_helper(container, plugin_space, callback, NULL, NULL);
}

int composite_unplug_callback(struct composite_plugin*container, int plugin_id, int (*callback)(aroop_txt_t*input, aroop_txt_t*output)) {
	// TODO FILLME
}

int composite_plug_bridge(struct composite_plugin*container, aroop_txt_t*plugin_space, int (*bridge)(int signature, void*x)) {
	composite_plug_helper(container, plugin_space, NULL, bridge, NULL);
}

int composite_unplug_bridge(struct composite_plugin*container, int plugin_id, int (*bridge)(int signature, void*x)) {
	// TODO FILLME
}

int composite_plug_inner_composite(struct composite_plugin*container, aroop_txt_t*plugin_space, struct composite_plugin*inner) {
	composite_plug_helper(container, plugin_space, NULL, NULL, inner);
}

int composite_unplug_inner_composite(struct composite_plugin*container, int plugin_id, struct composite_plugin*inner) {
	// TODO FILLME
}

int composite_call(struct composite_plugin*container, aroop_txt_t*plugin_space, aroop_txt_t*input, aroop_txt_t*output) {
	struct internal_plugin*plugin = opp_hash_table_get(&container->table, plugin_space);
	if(plugin == NULL)
		return 0;
	return plugin->x.callback(input, output);
}

C_CAPSULE_END
