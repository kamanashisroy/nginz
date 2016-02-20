
#include <aroop/aroop_core.h>
#include <aroop/aroop_memory_profiler.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "net/streamio.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/user.h"
#include "net/chat/profiler.h"

C_CAPSULE_START

int chat_profiler_plug_write_cb(void*log_data, aroop_txt_t*content) {
	if(aroop_txt_is_empty_magical(content))
		return 0;
	aroop_txt_t*buffer = (aroop_txt_t*)log_data;
	aroop_txt_concat(buffer, content);
	return 0;
}

static int chat_profiler_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(!IS_VALID_CHAT(chat)) // sanity check
		return 0;
	aroop_txt_t output = {};
	aroop_txt_embeded_stackbuffer(&output, 2048);
	aroop_write_output_stream_t strm = {.cb_data = &output, .cb = chat_profiler_plug_write_cb};
	aroop_memory_profiler_dump(strm, NULL, 1);
	aroop_txt_concat_char(&output, '\n');
	chat->strm.send(&chat->strm, &output, 0);
	return 0;
}

static int chat_profiler_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "profiler", "chat", plugin_space, __FILE__, "It shows the memory usage of the server.\n");
}

int chat_profiler_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/profiler");
	composite_plug_bridge(chat_plugin_manager_get(), &plugin_space, chat_profiler_plug, chat_profiler_plug_desc);
}

int chat_profiler_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_profiler_plug);
}


C_CAPSULE_END
