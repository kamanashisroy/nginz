
#include <time.h>
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "event_loop.h"
#include "streamio.h"
#include "chat.h"
#include "chat/chat_plugin_manager.h"
#include "chat/uptime.h"

C_CAPSULE_START


static int chat_uptime_get_fibers(aroop_txt_t*output) {
	aroop_txt_t req = {};
	aroop_txt_t resp = {};
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/fiber");
	pm_call(&plugin_space, &req, &resp); // needs cleanup
	aroop_txt_concat(output, &resp);
	aroop_txt_concat_char(output, '\n');
	aroop_txt_destroy(&req);
	aroop_txt_destroy(&resp); // cleanup
	return 0;
}

static long internal_chat_connection_count = 0;
static time_t start_time;
static int chat_uptime_plug(int signature, void*given) {
	aroop_assert(signature == CHAT_SIGNATURE);
	struct chat_connection*chat = (struct chat_connection*)given;
	if(!IS_VALID_CHAT(chat)) // sanity check
		return 0;
	aroop_txt_t uptime = {};
	aroop_txt_embeded_stackbuffer(&uptime, 256);
	time_t cur_time;
	time(&cur_time);
	double diff = difftime(cur_time, start_time);
	int hour = (int)(diff/3600);
	int minute = (int)(diff/60 - hour*60);
	int second = (int)(diff - hour*3600 - minute*60);
	aroop_txt_printf(&uptime, "%2d:%2d:%2d, %d users, total %ld users served, pid %d, ", hour, minute, second, event_loop_fd_count(), internal_chat_connection_count, getpid());
	chat_uptime_get_fibers(&uptime);
	chat->strm.send(&chat->strm, &uptime, 0);
	return 0;
}

static int chat_uptime_plug_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "uptime", "chat", plugin_space, __FILE__, "It shows the process uptime and load.\n");
}

static struct chat_connection*(*real_on_create)(int fd) = NULL;
static struct chat_connection*uptime_count_chat_create(int fd) {
	internal_chat_connection_count++;
	return real_on_create(fd);
}

int uptime_module_init() {
	time(&start_time);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "chat/uptime");
	cplug_bridge(chat_plugin_manager_get(), &plugin_space, chat_uptime_plug, chat_uptime_plug_desc);
	real_on_create = chat_api_get()->on_create;
	chat_api_get()->on_create = uptime_count_chat_create;
	return 0;
}

int uptime_module_deinit() {
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_uptime_plug);
	return 0;
}


C_CAPSULE_END
