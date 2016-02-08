#ifndef DEMOCHAT_PLUGIN_H
#define DEMOCHAT_PLUGIN_H

C_CAPSULE_START


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
int composite_plug_callback(struct composite_plugin*container, aroop_txt_t*plugin_space, int (*callback)(aroop_txt_t*input, aroop_txt_t*output), int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output));
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
int composite_plug_bridge(struct composite_plugin*container, aroop_txt_t*target, int (*bridge)(int signature, void*x), int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output));
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
int composite_plug_inner_composite(struct composite_plugin*container, aroop_txt_t*target, struct composite_plugin*x, int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output));
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

C_CAPSULE_END

#endif // DEMOCHAT_PLUGIN_H
