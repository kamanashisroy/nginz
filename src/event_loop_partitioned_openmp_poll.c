
#include "aroop/aroop_core.h"
#include "aroop/core/xtring.h"
#include "nginz_config.h"
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "event_loop.h"
#include "fiber.h"

C_CAPSULE_START

struct event_callback {
	int is_valid;
	int (*on_event)(int fd, int returned_events, const void*event_data);
#ifdef NGINZ_EVENT_DEBUG
	int (*on_debug)(int fd, const void*debug_data);
#endif
	const void*event_data;
};

#define POLL_PARTITION 8

static struct pollfd internal_fds[POLL_PARTITION][MAX_POLL_FD/POLL_PARTITION];
static struct event_callback internal_callback[POLL_PARTITION][MAX_POLL_FD/POLL_PARTITION];
static int internal_nfds[POLL_PARTITION] = {0,0,0,0};
static int grand_nfds = 0;


NGINZ_INLINE int event_loop_fd_count() {
#if 0
	int count = 0;
	int i = 0;
	for(i = 0; i < POLL_PARTITION; i++) {
		count += internal_nfds;
	}
	return count;
#else
	return grand_nfds;
#endif
}

int event_loop_register_fd(int fd, int (*on_event)(int fd, int returned_events, const void*event_data), const void*event_data, short requested_events) {
	aroop_assert(event_loop_fd_count() < MAX_POLL_FD); // it should not be assert
	int part = fd%POLL_PARTITION;
	internal_fds[part][internal_nfds[part]].fd = fd;
	internal_fds[part][internal_nfds[part]].events = requested_events;
	internal_fds[part][internal_nfds[part]].revents = 0; // make sure we continue the event_loop without any conflict
	internal_callback[part][internal_nfds[part]].is_valid = 1;
	internal_callback[part][internal_nfds[part]].on_event = on_event;
	internal_callback[part][internal_nfds[part]].event_data = event_data;
	internal_nfds[part]++;
	grand_nfds++;
}

#ifdef NGINZ_EVENT_DEBUG
int event_loop_register_debug(int fd, int (*on_debug)(int fd, const void*debug_data)) {
	int i = 0;
	int part = fd%POLL_PARTITION;
	for(i = 0; i < internal_nfds[part]; i++) {
		if(internal_fds[part][i].fd == fd) {
			internal_callback[part][i].on_debug = on_debug;
		}
	}
}

static int event_loop_debug() {
	int i = 0;
	int part = 0;
	for(part = 0; part < POLL_PARTITION; part++) {
		for(i = 0; i < internal_nfds[part]; i++) {
			if(internal_callback[part][i].is_valid && internal_callback[part][i].on_debug)
				internal_callback[part][i].on_debug(internal_fds[part][i].fd, internal_callback[part][i].event_data);
		}
	}
}
#endif

int event_loop_unregister_fd(int fd) {
	int i = 0;
	int part = fd%POLL_PARTITION;
	for(i = 0; i < internal_nfds[part]; i++) {
		if(internal_fds[part][i].fd != fd)
			continue;
		if(!internal_callback[part][i].is_valid)
			continue;
		internal_callback[part][i].is_valid = 0;
		break;
	
	}
	return 0;
}


int event_loop_batch_unregister(int part) {
	int i = 0;
	for(i = 0; i < internal_nfds[part]; i++) {
		if(internal_callback[part][i].is_valid)
			continue;
#ifdef NGINZ_EVENT_DEBUG
		event_loop_debug();
#endif
		if((internal_nfds[part] - i - 1) > 0) {
			memmove(internal_fds[part]+i, internal_fds[part]+i+1, sizeof(struct pollfd)*(internal_nfds[part]-i-1));
			memmove(internal_callback[part]+i, internal_callback[part]+i+1, sizeof(internal_callback[0][0])*(internal_nfds[part]-i-1));
		}
		internal_nfds[part]--;
#ifdef NGINZ_EVENT_DEBUG
		event_loop_debug();
#endif
		i--;
	
	}
	return 0;
}


static int event_loop_step_helper(int part, int count) {
	int i = 0;
	for(i = 0; i < internal_nfds[part] && count; i++) {
		if(!internal_callback[part][i].is_valid || !internal_fds[part][i].revents) {
			continue;
		}
		count--;
		if(internal_callback[part][i].on_event(internal_fds[part][i].fd, internal_fds[part][i].revents, internal_callback[part][i].event_data)) {
#ifdef NGINZ_EVENT_DEBUG
			event_loop_debug();
#endif
			// fd is closed and may be removed.
			//return event_loop_step_helper(count); // start over
			// it is OK
		}
	}
	return 0;
}

static int event_loop_step(int status) {
	if(event_loop_fd_count() == 0) {
		usleep(1000);
		return 0;
	}
	int count[POLL_PARTITION];
	int part = 0;
	for(part = 0; part < POLL_PARTITION; part++) {
		count[part] = 0;
	}
	#pragma omp for
	for(part = 0; part < POLL_PARTITION; part++) {
		if(!internal_nfds[part])
			continue;
		if(!(count[part] = poll(internal_fds[part], internal_nfds[part], 100)))
			continue;
		if(count[part] == -1) {
			syslog(LOG_ERR, "event_loop.c poll failed %s", strerror(errno));
			aroop_assert(count[part] != -1);
		}
		#pragma omp critical
		event_loop_step_helper(part, count[part]);
		event_loop_batch_unregister(part);
	}
	// fix grand total
	grand_nfds = 0;
	for(part = 0; part < POLL_PARTITION; part++) {
		grand_nfds += internal_nfds[part];
	}
	return 0;
}

#if 0
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
#endif

int event_loop_module_init() {
	int i = 0;
	for(i = 0; i < POLL_PARTITION; i++) {
		internal_nfds[i] = 0;
	}
	register_fiber(event_loop_step);
#if 0
	aroop_txt_t plugin_space;
	aroop_txt_embeded_set_static_string(&plugin_space, "test/event_loop_test");
	pm_plug_callback(&plugin_space, event_loop_test, event_loop_test_desc);
#endif
}

int event_loop_module_deinit() {
	unregister_fiber(event_loop_step);
#if 0
	pm_unplug_callback(0, event_loop_test);
#endif
}

C_CAPSULE_END
