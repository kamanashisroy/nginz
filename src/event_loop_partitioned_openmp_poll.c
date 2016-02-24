
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
	const void*event_data;
};

enum {
	POLL_PARTITION = (1<<3),
	MAX_POLL_FD_PER_PARTITION = (MAX_POLL_FD/POLL_PARTITION),
};

static struct event_loop_data {
	struct pollfd fds[MAX_POLL_FD_PER_PARTITION];
	struct event_callback cbs[MAX_POLL_FD_PER_PARTITION];
	int count;
} edata[POLL_PARTITION];
static int grand_nfds = 0;


NGINZ_INLINE int event_loop_fd_count() {
	return grand_nfds;
}

int event_loop_register_fd(int fd, int (*on_event)(int fd, int returned_events, const void*event_data), const void*event_data, short requested_events) {
	aroop_assert(event_loop_fd_count() < MAX_POLL_FD); // it should not be assert
	const int part = fd%POLL_PARTITION;
	const int index = edata[part].count;
	edata[part].fds[index].fd = fd;
	edata[part].fds[index].events = requested_events;
	edata[part].fds[index].revents = 0; // make sure we continue the event_loop without any conflict
	edata[part].cbs[index].is_valid = 1;
	edata[part].cbs[index].on_event = on_event;
	edata[part].cbs[index].event_data = event_data;
	edata[part].count++;
	grand_nfds++;
}

int event_loop_unregister_fd(const int fd) {
	int i = 0;
	const int part = fd%POLL_PARTITION;
	const int count = edata[part].count;
	for(i = 0; i < count; i++) {
		if(edata[part].fds[i].fd != fd)
			continue;
		if(!edata[part].cbs[i].is_valid)
			continue;
		edata[part].cbs[i].is_valid = 0; // lazy unregister
		break;
	}
	return 0;
}


int event_loop_batch_unregister(struct event_loop_data*const pt) {
	int i = 0;
	int top = pt->count - 1;
	// trim the last invalid elements
	for(; top >= 0; top--) {
		if(pt->cbs[top].is_valid)
			break;
	}
	// now we swap the invalid with top
	for(i=0; i < top; i++) {
		if(pt->cbs[i].is_valid)
			continue;
		pt->fds[i] = pt->fds[top];
		pt->cbs[i] = pt->cbs[top];
		top--;
	}
	pt->count = top+1;
	return 0;
}


static int event_loop_step_helper(struct event_loop_data*const pt, int count) {
	int i = 0;
	for(i = 0; i < pt->count && count; i++) {
		if(!pt->cbs[i].is_valid || !pt->fds[i].revents) {
			continue;
		}
		count--;
		pt->cbs[i].on_event(pt->fds[i].fd, pt->fds[i].revents, pt->cbs[i].event_data);
	}
	return 0;
}

static int event_loop_step(int status) {
	if(event_loop_fd_count() == 0) {
		usleep(1000);
		return 0;
	}
	int ecount = 0;
	int part = 0;
	#pragma omp for private(ecount)
	for(part = 0; part < POLL_PARTITION; part++) {
		if(!edata[part].count)
			continue;
		if(!(ecount = poll(edata[part].fds, edata[part].count, 100)))
			continue;
		if(ecount == -1) {
			syslog(LOG_ERR, "event_loop.c poll failed %s", strerror(errno));
			aroop_assert(ecount != -1);
		}
		#pragma omp critical
		event_loop_step_helper(edata+part, ecount);
		event_loop_batch_unregister(edata+part);
	}
	// fix grand total
	grand_nfds = 0;
	for(part = 0; part < POLL_PARTITION; part++) {
		grand_nfds += edata[part].count;
	}
	return 0;
}

int event_loop_module_init() {
	int i = 0;
	for(i = 0; i < POLL_PARTITION; i++) {
		edata[i].count = 0;
	}
	grand_nfds = 0;
	register_fiber(event_loop_step);
}

int event_loop_module_deinit() {
	unregister_fiber(event_loop_step);
}

C_CAPSULE_END
