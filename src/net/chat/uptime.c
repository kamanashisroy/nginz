
#include <time.h>
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "net/chat.h"
#include "net/chat/chat_plugin_manager.h"
#include "net/chat/uptime.h"

C_CAPSULE_START

static time_t start_time;
static int chat_uptime_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(chat == NULL || chat->fd == -1) // sanity check
		return 0;
	aroop_txt_t uptime = {};
	aroop_txt_embeded_stackbuffer(&uptime, 256);
	time_t cur_time;
	time(&cur_time);
	double diff = difftime(cur_time, start_time);
	int hour = (int)(diff/3600);
	int minute = (int)(diff/60 - hour*60);
	int second = (int)(diff - hour*3600 - minute*60);
	aroop_txt_printf(&uptime, "%2d:%2d:%2d, %d users, %d\n", hour, minute, second, event_loop_fd_count(), getpid());
	send(chat->fd, aroop_txt_to_string(&uptime), aroop_txt_length(&uptime), 0);
	return 0;
}

static int chat_uptime_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "uptime", "chat", plugin_space, __FILE__, "It shows the process uptime and load.\n");
}

int uptime_module_init() {
	time(&start_time);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/uptime");
	composite_plug_bridge(chat_plugin_manager_get(), &plugin_space, chat_uptime_plug, chat_uptime_plug_desc);
}

int uptime_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_uptime_plug);
}


C_CAPSULE_END
