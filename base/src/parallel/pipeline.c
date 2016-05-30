
#include <sys/socket.h>
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include <aroop/aroop_memory_profiler.h>
#include "nginz_config.h"
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "event_loop.h"
#include "binary_coder.h"
#include "parallel/pipeline.h"
#include "parallel/async_request.h"
#include "parallel/ping.h"
#include "parallel/async_request_internal.h"

/****************************************************/
/********* This file is really tricky ***************/
/****************************************************/
C_CAPSULE_START

/**
 * This is node structure. It contains the fd pipes.
 */
struct nginz_node {
	int nid;
	int fd[2]; /* @brief rule of thumb, write in fd[0], read from fd[1] */
	int raw_fd[2]; /* @brief this fd is for raw messaging(normally used to send socket fd) */
};

enum {
	MAX_PROCESS_COUNT = NGINZ_NUMBER_OF_PROCESSORS+1,
};
/****************************************************/
static struct nginz_node nodes[MAX_PROCESS_COUNT];
static struct nginz_node*mynode = nodes;

NGINZ_INLINE static struct nginz_node*pp_find_node(int nid) {
	int i = 0;
	for(i = 0; i < MAX_PROCESS_COUNT; i++) {
		if(nodes[i].nid == nid)
			return (nodes+i);
	}
	return NULL; /* not found */
}

NGINZ_INLINE int pp_next_nid() {
	int i = 0;
	int next_index = -1;
	for(i = 0; i < MAX_PROCESS_COUNT; i++) {
		if((nodes+i) == mynode) {
			next_index = i+1;
			break;
		}
	}
	if(next_index != -1 && next_index < MAX_PROCESS_COUNT)
		return nodes[i].nid;
	return -1; /* not found */
}


NGINZ_INLINE static int pp_simple_sendmsg(int through, aroop_txt_t*pkt) {
	struct msghdr msg;
	struct iovec iov[1];
	memset(&msg, 0, sizeof(msg));
	memset(iov, 0, sizeof(iov));
	iov[0].iov_base = aroop_txt_to_string(pkt);
	iov[0].iov_len  = aroop_txt_length(pkt);
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	if(sendmsg(through, &msg, 0) < 0) {
		syslog(LOG_ERR, "Cannot send simple message to child:%s\n", strerror(errno));
		return -1;
	}
	return 0;
}

NGINZ_INLINE int pp_send(int dnid, aroop_txt_t*pkt) {
	struct nginz_node*nd = NULL;
	int i = 0;
	// sanity check
	if(dnid == 0) { /* broadcast */
		for(i = 0; i < MAX_PROCESS_COUNT; i++) {
			if((nodes+i) != mynode && pp_simple_sendmsg(nodes[i].fd[0], pkt))
				return -1;
		}
	} else {
		nd = pp_find_node(dnid);
		aroop_assert(nd);
		return pp_simple_sendmsg(nd->fd[0], pkt);
	}
	return 0;
}

NGINZ_INLINE int is_master() {
	return (mynode && mynode == nodes);
}

/****************************************************/
/********** Pipe event listeners ********************/
/****************************************************/

NGINZ_INLINE static int pp_simple_recvmsg_helper(int through, aroop_txt_t*cmd) {
	struct msghdr msg;
	struct iovec iov[1];
	if(through == -1)
		return -1;
	memset(&msg, 0, sizeof(msg));
	memset(iov, 0, sizeof(iov));
	iov[0].iov_base = aroop_txt_to_string(cmd);
	iov[0].iov_len  = aroop_txt_capacity(cmd);
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	
	int recvlen = 0;
	if((recvlen = recvmsg(through, &msg, 0)) < 0) {
		syslog(LOG_ERR, "Cannot recv msg:%s\n", strerror(errno));
		return -1;
	}
	if(msg.msg_iovlen == 1 && iov[0].iov_len > 0) {
		aroop_txt_set_length(cmd, iov[0].iov_len);
	}
	return 0;
}


static aroop_txt_t recv_buffer;
static int on_bubbles(int fd, int events, const void*unused) {
	aroop_assert(mynode && (fd == mynode->fd[1]));
	if(NGINZ_POLL_CLOSE_FLAGS == (events & NGINZ_POLL_CLOSE_FLAGS)) {
		// TODO check if it happens
		syslog(LOG_ERR, "Somehow the pipe is closed\n");
		event_loop_unregister_fd(fd);
		close(fd);
		return 0;
	}
	//printf("There is bubble_down from the parent\n");
	aroop_txt_set_length(&recv_buffer, 1); // without it aroop_txt_to_string() will give NULL
	//char rbuf[255];
	//int count = recv(parent, rbuf, sizeof(rbuf), 0);
	pp_simple_recvmsg_helper(fd, &recv_buffer);
	if(aroop_txt_is_empty(&recv_buffer)) {
		syslog(LOG_ERR, "Error receiving bubble_down:%s\n", strerror(errno));
		close(fd);
		return 0;
	}

#if 1
	// TODO use the srcpid for something ..
	//aroop_txt_embeded_rebuild_and_set_content(&recv_buffer, rbuf)
	int srcpid = 0;
	//printf("There is bubble_down from the parent, %d, (count=%d)\n", (int)aroop_txt_char_at(&recv_buffer, 0), count);
	binary_unpack_int(&recv_buffer, 0, &srcpid);
#endif
	//syslog(LOG_NOTICE, "[pid:%d]\treceiving from parent for %d", getpid(), destpid);
	aroop_txt_t x = {};
	binary_unpack_string(&recv_buffer, 1, &x); // needs cleanup
	do {
		if(aroop_txt_is_empty(&x)) {
			break;
		}
		aroop_txt_t output = {};
		pm_call(&x, &recv_buffer, &output);
		aroop_txt_destroy(&output);
	} while(0);
	//syslog(LOG_NOTICE, "[pid:%d]\texecuting command:%s", getpid(), aroop_txt_to_string(&x));
	aroop_txt_destroy(&x);
	return 0;
}


/****************************************************/
/********** Fork event listeners ********************/
/****************************************************/

static int pp_fork_child_after_callback(aroop_txt_t*input, aroop_txt_t*output) {
	int i = 0;
	/****************************************************/
	/********* Cleanup old parent fds *******************/
	/****************************************************/
	mynode = NULL;
	for(i = 0; i < MAX_PROCESS_COUNT; i++) {
		if(!mynode && !nodes[i].nid) {
			mynode = (nodes+i);
			close(nodes[i].fd[0]);
			close(nodes[i].raw_fd[0]);
			mynode->nid = getpid();
		} else {
			close(nodes[i].fd[1]);
			close(nodes[i].raw_fd[1]);
		}
	}
	/****************************************************/
	/********* Register readers *************************/
	/****************************************************/
	event_loop_register_fd(mynode->fd[1], on_bubbles, NULL, NGINZ_POLL_ALL_FLAGS);
	return 0;
}

static int pp_fork_parent_after_callback(aroop_txt_t*input, aroop_txt_t*output) {
	int i = 0;
	for(i = 0; i < MAX_PROCESS_COUNT; i++) {
		if(!nodes[i].nid) {
			nodes[i].nid = 1; /* TODO set child pid */
			break;
		}
	}
	/* check if the forking is all complete */
	if(nodes[MAX_PROCESS_COUNT-1].nid) { /* if there is no more forking cleanup */
		for(i = 1/* skip the master */; i < MAX_PROCESS_COUNT; i++) {
			/* close the read fd */
			close(nodes[i].fd[1]);
			close(nodes[i].raw_fd[1]);
		}
		/****************************************************/
		/********* Register readers *************************/
		/****************************************************/
		event_loop_register_fd(mynode->fd[1], on_bubbles, NULL, NGINZ_POLL_ALL_FLAGS);
	}
	return 0;
}

static int pp_fork_callback_desc(aroop_txt_t*plugin_space,aroop_txt_t*output) {
	return plugin_desc(output, "pipeline", "fork", plugin_space, __FILE__, "It allows the processes to pipeline messages to and forth.\n");
}

/****************************************************/
/****** Module constructors and destructors *********/
/****************************************************/
int pp_module_init() {
	memset(nodes, 0, sizeof(nodes));
	int i = 0;
	/* make all the pipes beforehand, so that all of the nodes know each other */
	for(i = 0; i < MAX_PROCESS_COUNT; i++) {
		if(socketpair(AF_UNIX, SOCK_DGRAM, 0, nodes[i].fd) || socketpair(AF_UNIX, SOCK_DGRAM, 0, nodes[i].raw_fd)) {
			syslog(LOG_ERR, "Failed to create pipe:%s\n", strerror(errno));
			return -1;
		}
	}
	mynode->nid = getpid();
	aroop_txt_embeded_buffer(&recv_buffer, NGINZ_MAX_BINARY_MSG_LEN);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/child/after");
	pm_plug_callback(&plugin_space, pp_fork_child_after_callback, pp_fork_callback_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/parent/after");
	pm_plug_callback(&plugin_space, pp_fork_parent_after_callback, pp_fork_callback_desc);
	ping_module_init();
	async_request_init();
	return 0;
}

int pp_module_deinit() {
	async_request_deinit();
	ping_module_deinit();
	// TODO unregister all
	aroop_txt_destroy(&recv_buffer);
	return 0;
}

C_CAPSULE_END
