
#include "aroop/aroop_core.h"
#include "aroop/core/xtring.h"
#include "nginz_config.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "event_loop.h"
#include "fiber.h"

C_CAPSULE_START

struct event_callback {
	int (*on_event)(int fd, int returned_events, const void*event_data);
#ifdef NGINZ_EVENT_DEBUG
	int (*on_debug)(int fd, const void*debug_data);
#endif
	const void*event_data;
};

static struct pollfd internal_fds[MAX_POLL_FD];
static struct event_callback internal_callback[MAX_POLL_FD];
static int internal_nfds = 0;


int event_loop_fd_count() {
	return internal_nfds;
}

int event_loop_register_fd(int fd, int (*on_event)(int fd, int returned_events, const void*event_data), const void*event_data, short requested_events) {
	aroop_assert(internal_nfds < MAX_POLL_FD);
	internal_fds[internal_nfds].fd = fd;
	internal_fds[internal_nfds].events = requested_events;
	internal_callback[internal_nfds].on_event = on_event;
	internal_callback[internal_nfds].event_data = event_data;
	internal_nfds++;
}

#ifdef NGINZ_EVENT_DEBUG
int event_loop_register_debug(int fd, int (*on_debug)(int fd, const void*debug_data)) {
	int i = 0;
	for(i = 0; i < internal_nfds; i++) {
		if(internal_fds[i].fd == fd) {
			internal_callback[i].on_debug = on_debug;
		}
	}
}

static int event_loop_debug() {
	int i = 0;
	for(i = 0; i < internal_nfds; i++) {
		if(internal_callback[i].on_debug)
			internal_callback[i].on_debug(internal_fds[i].fd, internal_callback[i].event_data);
	}
}
#endif

int event_loop_unregister_fd(int fd) {
	int i = 0;
	for(i = 0; i < internal_nfds; i++) {
		if(internal_fds[i].fd == fd) {
#ifdef NGINZ_EVENT_DEBUG
			event_loop_debug();
#endif
			if((internal_nfds - i - 1) > 0) {
				memmove(internal_fds+i, internal_fds+i+1, sizeof(struct pollfd)*(internal_nfds-i-1));
				memmove(internal_callback+i, internal_callback+i+1, sizeof(internal_callback[0])*(internal_nfds-i-1));
			}
			internal_nfds--;
#ifdef NGINZ_EVENT_DEBUG
			event_loop_debug();
#endif
			return 0;
		}
	}
	return 0;
}

static int event_loop_step_helper(int count) {
	int i = 0;
	for(i = 0; i < internal_nfds && count; i++) {
		if(!internal_fds[i].revents) {
			continue;
		}
		count--;
		if(internal_callback[i].on_event(internal_fds[i].fd, internal_fds[i].revents, internal_callback[i].event_data)) {
#ifdef NGINZ_EVENT_DEBUG
			event_loop_debug();
#endif
			// fd is closed and may be removed.
			return event_loop_step_helper(count); // start over
		}
	}
	return 0;
}

static int event_loop_step(int status) {
	if(internal_nfds == 0) {
		usleep(1000);
		return 0;
	}
	int count = 0;
	if(!(count = poll(internal_fds, internal_nfds, 100)))
		return 0;
	return event_loop_step_helper(count);
}

static int event_loop_test_helper(int count) {
	int fd = 5000;
	int ncount = count;
	int prev = event_loop_fd_count();
	while(ncount--) {
		event_loop_register_fd(fd+ncount, NULL, NULL, POLLIN);
	}
	ncount = count;
	while(ncount--) {
		event_loop_unregister_fd(fd+ncount);
	}
	
	return !(prev == event_loop_fd_count());
}

static int event_loop_test(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_embeded_buffer(output, 512);
	if(event_loop_test_helper(1000)) {
		aroop_txt_printf(output, "event_loop.c:FAILED\n");
	} else {
		aroop_txt_concat_string(output, "event_loop.c:successful\n");
	}
	return 0;
}

static int event_loop_test_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "event_loop_test", "test", plugin_space, __FILE__, "It is test code for event loop.\n");
}

int event_loop_module_init() {
	register_fiber(event_loop_step);
	aroop_txt_t plugin_space;
	aroop_txt_embeded_set_static_string(&plugin_space, "test/event_loop_test");
	pm_plug_callback(&plugin_space, event_loop_test, event_loop_test_desc);
}

int event_loop_module_deinit() {
	unregister_fiber(event_loop_step);
	pm_unplug_callback(0, event_loop_test);
}

C_CAPSULE_END
