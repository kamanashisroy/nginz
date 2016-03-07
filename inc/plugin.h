#ifndef NGINZ_PLUGIN_H
#define NGINZ_PLUGIN_H

C_CAPSULE_START


enum plugin_category {
	CALLBACK_PLUGIN = 0,
	COMPOSITE_PLUGIN,
	BRIDGE_PLUGIN,
	INVALID_PLUGIN,
};

/**
 * Composite plugin registers a group of plugin under it.
 */
struct composite_plugin;

/**
 * @usage It creates an extension
 * @param container that holds the plugin
 * @param the name-space of the plugin
 * @param the callback function
 * @return plugin_id
 */
int composite_plug_callback(struct composite_plugin*container, aroop_txt_t*plugin_space, int (*callback)(aroop_txt_t*input, aroop_txt_t*output), int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output), const char*filename, int lineno);
#define cplug_callback(container, plugin_space, callback, desc) composite_plug_callback(container, plugin_space, callback, desc, __FILE__, __LINE__)
/**
 * @usage It unregisteres the extension by plugin_id
 * @param container that holds the plugin
 * @param the plugin_id of the plugin
 */
int composite_unplug_callback(struct composite_plugin*container, int plugin_id, int (*callback)(aroop_txt_t*input, aroop_txt_t*output));

/**
 * @usage it registers a bridge plugin
 * @param container that holds the plugin
 * @param the name-space of the plugin
 * @param the bridge function
 */
int composite_plug_bridge(struct composite_plugin*container, aroop_txt_t*target, int (*bridge)(int signature, void*x), int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output), const char*filename, int lineno);
#define cplug_bridge(container, plugin_space, bridge, desc) composite_plug_bridge(container, plugin_space, bridge, desc, __FILE__, __LINE__)
/**
 * @usage it unregisters a bridge plugin
 * @param container that holds the plugin
 * @param the plugin-id
 * @param the bridge function
 */
int composite_unplug_bridge(struct composite_plugin*container, int plugin_id, int (*bridge)(int signature, void*x));

/**
 * @usage it registers innter composite plugin
 * @param container that holds the plugin
 * @param the name-space of the plugin
 * @param the composite
 */
int composite_plug_inner_composite(struct composite_plugin*container, aroop_txt_t*target, struct composite_plugin*x, int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output), const char*filename, int lineno);
#define cplug_inner(container, plugin_space, inner, desc) composite_plug_bridge(container, plugin_space, inner, desc, __FILE__, __LINE__)
/**
 * @usage it unregisters innter composite plugin
 * @param container that holds the plugin
 * @param the plugin-id
 * @param the composite
 */
int composite_unplug_inner_composite(struct composite_plugin*container, int plugin_id, struct composite_plugin*x);

int composite_plugin_call(struct composite_plugin*container, aroop_txt_t*plugin_space, aroop_txt_t*input, aroop_txt_t*output);

int composite_plugin_visit_all(struct composite_plugin*container, int (*visitor)(
		int category
		, aroop_txt_t*plugin_space
		, int(*callback)(aroop_txt_t*input, aroop_txt_t*output)
		, int(*bridge)(int signature, void*x)
		, struct composite_plugin*inner
		, int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output)
		, void*visitor_data
	), void*visitor_data);

int plugin_desc(aroop_txt_t*output, char*plugin_name, char*plugin_type, aroop_txt_t*space, char*source_file, char*desc);
int composite_plugin_test(struct composite_plugin*container);

C_CAPSULE_END

#endif // NGINZ_PLUGIN_H
