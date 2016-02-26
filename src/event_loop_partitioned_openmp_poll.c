
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
	int nfds;
} edata[POLL_PARTITION];
static int grand_nfds = 0;


NGINZ_INLINE int event_loop_fd_count() {
	return grand_nfds;
}

int event_loop_register_fd(int fd, int (*on_event)(int fd, int returned_events, const void*event_data), const void*event_data, short requested_events) {
	aroop_assert(event_loop_fd_count() < MAX_POLL_FD); // it should not be assert
	const int part = fd%POLL_PARTITION;
	struct event_loop_data*const pt = edata+part;
	const int index = pt->nfds;
	pt->fds[index].fd = fd;
	pt->fds[index].events = requested_events;
	pt->fds[index].revents = 0; // make sure we continue the event_loop without any conflict
	pt->cbs[index].on_event = on_event;
	pt->cbs[index].event_data = event_data;
	pt->nfds++;
	grand_nfds++;
	return 0;
}

int event_loop_unregister_fd(const int fd) {
	int i = 0;
	struct event_loop_data*const pt = edata+(fd%POLL_PARTITION);
	const int count = pt->nfds;
	for(i = 0; i < count; i++) {
		if(pt->fds[i].fd != fd)
			continue;
		if(!pt->fds[i].events)
			continue;
		pt->fds[i].events = 0; // lazy unregister
		break;
	}
	return 0;
}

/**
 * This algorithm converts the sparse array to a compact one. It is done in O(n) running time.
 */
static int event_loop_batch_unregister(struct event_loop_data*const pt) {
	int i = 0;
	int last = pt->nfds - 1;
	// trim the last invalid elements
	for(; last >= 0 && !pt->fds[last].events; last--);
	// now we swap the invalid with last
	for(i=0; i < last; i++) {
		if(pt->fds[i].events)
			continue;
		pt->fds[i] = pt->fds[last];
		pt->cbs[i] = pt->cbs[last];
		last--;
		// trim the last invalid elements
		for(; last >= 0 && !pt->fds[last].events; last--);
	}
	pt->nfds = last+1;
	return 0;
}


static int event_loop_step_helper(struct event_loop_data*const pt, int ecount) {
	int i = 0;
	for(i = 0; i < pt->nfds && ecount; i++) {
		if(!pt->fds[i].events || !pt->fds[i].revents)
			continue;
		ecount--;
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
	//#pragma omp for private(ecount)
	for(part = 0; part < POLL_PARTITION; part++) {
		if(!edata[part].nfds)
			continue;
		if(!(ecount = poll(edata[part].fds, edata[part].nfds, 100)))
			continue;
		if(ecount == -1) {
			syslog(LOG_ERR, "event_loop.c poll failed %s", strerror(errno));
			aroop_assert(ecount != -1);
		}
		//#pragma omp critical
		event_loop_step_helper(edata+part, ecount);
		event_loop_batch_unregister(edata+part);
	}
	// fix grand total
	grand_nfds = 0;
	for(part = 0; part < POLL_PARTITION; part++) {
		grand_nfds += edata[part].nfds;
	}
	return 0;
}

#define FAKE_FD_COUNT 1024
static int event_loop_test_event_cb(int fd, int returned_events, const void*event_data) {
	return 0;
}

static int event_loop_test_exists_fd(int fd) {
	struct event_loop_data*const pt = edata+(fd%POLL_PARTITION);
	int i = 0;
	const int count = pt->nfds;
	for(i = 0 ; i < count; i++) {
		if(!pt->fds[i].events)
			continue;
		if(pt->fds[i].fd == fd)
			return 1;
	}
	return 0;
}

static int event_loop_test_helper() {
	int fake_fds[FAKE_FD_COUNT];
	int removed_fake_fds[FAKE_FD_COUNT];
	int i = 0, j = 0;
	int mycount = event_loop_fd_count();
	int firstcount = event_loop_fd_count();
	// add fake fds
	for(i = 0; i < FAKE_FD_COUNT; i++) {
		removed_fake_fds[i] = 0;
		fake_fds[i] = i + 100;
		event_loop_register_fd(fake_fds[i], event_loop_test_event_cb, NULL, NGINZ_POLL_ALL_FLAGS);
	}
	mycount += FAKE_FD_COUNT;
	if(event_loop_fd_count() != mycount) {
		syslog(LOG_ERR, "event_loop.....c:we could not add all the test cases");
		return -1;
	}
	// remove some
	for(i = 0; i < 100; i++) {
		int intval = (rand()%FAKE_FD_COUNT);
		if(removed_fake_fds[intval]) 
			continue;
		event_loop_unregister_fd(fake_fds[i]);
		removed_fake_fds[i] = 1;
		mycount--;
	}
	// really remove
	int part;
	for(part = 0; part < POLL_PARTITION; part++) {
		event_loop_batch_unregister(edata+part);
	}
	grand_nfds = 0;
	for(part = 0; part < POLL_PARTITION; part++) {
		grand_nfds += edata[part].nfds;
	}
	if(event_loop_fd_count() != mycount) {
		syslog(LOG_ERR, "event_loop.....c:really removed and then count does not match:(%d,%d)", event_loop_fd_count(), mycount);
		return -1;
	}
	// check integrity
	for(i = 0; i < FAKE_FD_COUNT; i++) {
		if(removed_fake_fds[i]) {
			if(event_loop_test_exists_fd(fake_fds[i])) {
				syslog(LOG_ERR, "event_loop....c:removed but still there");
				return -1;
			}
		} else if(!event_loop_test_exists_fd(fake_fds[i])) {
			syslog(LOG_ERR, "event_loop....c:added but lost");
			return -1;
		}
	}
	for(i = 0; i < FAKE_FD_COUNT; i++) {
		if(!removed_fake_fds[i])
			event_loop_unregister_fd(fake_fds[i]);
	}
	// cleanup
	for(part = 0; part < POLL_PARTITION; part++) {
		event_loop_batch_unregister(edata+part);
	}
	grand_nfds = 0;
	for(part = 0; part < POLL_PARTITION; part++) {
		grand_nfds += edata[part].nfds;
	}
	if(firstcount != event_loop_fd_count()) {
		syslog(LOG_ERR, "event_loop....c:firstcount and lastcount does not match");
	}
	return 0;
}

static int event_loop_test(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_embeded_buffer(output, 512);
	aroop_txt_concat_string(output, "You should not run this test on loaded server\n");
	if(event_loop_test_helper()) {
		aroop_txt_concat_string(output, "event_loop.c:FAILED\n");
	} else {
		aroop_txt_concat_string(output, "event_loop.c:successful\n");
	}
	return 0;
}

static int event_loop_test_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "event_loop_test", "test", plugin_space, __FILE__, "It is test code for event loop.\n");
}

int event_loop_module_init() {
	int i = 0;
	for(i = 0; i < POLL_PARTITION; i++) {
		edata[i].nfds = 0;
	}
	grand_nfds = 0;
	register_fiber(event_loop_step);
	aroop_txt_t plugin_space;
	aroop_txt_embeded_set_static_string(&plugin_space, "test/event_loop_test");
	pm_plug_callback(&plugin_space, event_loop_test, event_loop_test_desc);
#if 0
	aroop_txt_t input = {};
	aroop_txt_t output = {};
	event_loop_test(&input, &output);
	aroop_txt_zero_terminate(&output);
	syslog(LOG_ERR, "eventloop test :%s", aroop_txt_to_string(&output));
	aroop_txt_destroy(&input);
	aroop_txt_destroy(&output);
#endif
	return 0;
}

int event_loop_module_deinit() {
	unregister_fiber(event_loop_step);
	pm_unplug_callback(0, event_loop_test);
	return 0;
}

C_CAPSULE_END
