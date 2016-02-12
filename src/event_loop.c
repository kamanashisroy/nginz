
#include "aroop/aroop_core.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "fiber.h"

C_CAPSULE_START

struct event_callback {
	int (*on_event)(int returned_events, const void*event_data);
	const void*event_data;
};

static struct pollfd internal_fds[MAX_POLL_FD];
static struct event_callback internal_callback[MAX_POLL_FD];
static int internal_nfds = 0;


int event_loop_fd_count() {
	return internal_nfds;
}

int event_loop_register_fd(int fd, int (*on_event)(int returned_events, const void*event_data), const void*event_data, short requested_events) {
	internal_fds[internal_nfds].fd = fd;
	internal_fds[internal_nfds].events = requested_events;
	internal_callback[internal_nfds].on_event = on_event;
	internal_callback[internal_nfds].event_data = event_data;
	internal_nfds++;
}

int event_loop_unregister_fd(int fd) {
	int i = 0;
	for(i = 0; i < internal_nfds; i++) {
		if(internal_fds[i].fd == fd) {
			if((internal_nfds - i - 1) > 0) {
				memcpy(internal_fds+i, internal_fds+i+1, sizeof(struct pollfd)*(internal_nfds-i-1));
				memcpy(internal_callback+i, internal_callback+i+1, sizeof(internal_callback[0])*(internal_nfds-i-1));
			}
			internal_nfds--;
			return 0;
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
	int i = 0;
	for(i = 0; i < internal_nfds && count; i++) {
		if(!internal_fds[i].revents) {
			continue;
		}
		count--;
		if(internal_callback[i].on_event(internal_fds[i].revents, internal_callback[i].event_data)) {
			// fd is closed and may be removed.
			return 0;
		}
	}
	return 0;
}

int event_loop_module_init() {
	register_fiber(event_loop_step);
}

int event_loop_module_deinit() {
	unregister_fiber(event_loop_step);
}

C_CAPSULE_END
