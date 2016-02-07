

#include "plugin.h"

C_CAPSULE_START

enum plugin_category {
	CALLBACK_PLUGIN = 0,
	COMPOSITE_PLUGIN,
	BRIDGE_PLUGIN,
}
struct composite_plugin {
	struct opp_factory*plugins;
}

struct internal_plugin {
	int category;
	union {
		int (callback*)(aroop_txt_t*input, aroop_txt_t*output);
		int (bridge*)(int signature, void*x);
		struct composite_plugin inner;
	} x;
};

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

int composite_plug_callback(struct composite_plugin*container, aroop_txt_t*plugin_space, int (callback*)(aroop_txt_t*input, aroop_txt_t*output)) {
	/**
	 * Create the fiber object and save that in a collection.
	 */
	struct internal_plugin plugin = OPP_ALLOC1(&container);
	OPP_ASSERT(plugin != NULL);
	plugin->category = CALLBACK_PLUGIN;
	plugin->x->callback = callback;
	// TODO register the callback in a hashtable
}

C_CAPSULE_END
