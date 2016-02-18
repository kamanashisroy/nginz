
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
#include "plugin_manager.h"
#include "event_loop.h"
#include "parallel/pipeline.h"
#include "parallel/ping.h"
#include "net/protostack.h"

struct connection {
	int fd;
	struct protostack*stack;
	void*pvt;
};
/****************************************************/
/********* This file is really tricky ***************/
/****************************************************/
C_CAPSULE_START

int child = -1;
int mchild = -1;
int parent = -1;
int mparent = -1;

NGINZ_INLINE int pp_bubble_down(aroop_txt_t*pkt) {
	// sanity check
	if(child == -1)
		return -1;
	send(child, aroop_txt_to_string(pkt), aroop_txt_length(pkt), 0);
	return 0;
}

NGINZ_INLINE static int pp_sendmsg_helper(int through, int target, aroop_txt_t*cmd) {
	union {
		int target;
		char buf[CMSG_SPACE(sizeof(int))];
	} intbuf;
	intbuf.target = target;
	struct msghdr msg;
	struct iovec iov[1];
	struct cmsghdr *control_message = NULL;
	memset(&intbuf, 0, sizeof(intbuf));
	// sanity check
	if(through == -1)
		return -1;
	memset(&msg, 0, sizeof(msg));
	memset(iov, 0, sizeof(iov));

	iov[0].iov_base = aroop_txt_to_string(cmd);
	iov[0].iov_len  = aroop_txt_length(cmd);
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_control = intbuf.buf;
	msg.msg_controllen = CMSG_SPACE(sizeof(int));
	control_message = CMSG_FIRSTHDR(&msg);
	control_message->cmsg_level = SOL_SOCKET;
	control_message->cmsg_type = SCM_RIGHTS;
	control_message->cmsg_len = CMSG_LEN(sizeof(int));
	*((int *) CMSG_DATA(control_message)) = target;
	//printf("Sending fd %d to worker\n", target);
	
	if(sendmsg(through, &msg, 0) < 0) {
		syslog(LOG_ERR, "Cannot send message to child:%s\n", strerror(errno));
		return -1;
	}
	close(target); // we do not own this fd anymore
	return 0;
}

NGINZ_INLINE int pp_bubble_down_send_socket(int socket, aroop_txt_t*cmd) {
	//printf("Sending fd %d to child\n", socket);
	return pp_sendmsg_helper(mchild, socket, cmd);
}

NGINZ_INLINE int pp_bubble_up(aroop_txt_t*pkt) {
	// sanity check
	if(parent == -1)
		return -1;
	send(parent, aroop_txt_to_string(pkt), aroop_txt_length(pkt), 0);
	return 0;
}

NGINZ_INLINE int pp_bubble_up_send_socket(int socket, aroop_txt_t*cmd) {
	if(mparent == -1)
		return -1;
	return pp_sendmsg_helper(mparent, socket, cmd);
}

NGINZ_INLINE int is_master() {
	return (parent == -1);
}

/****************************************************/
/********** Pipe event listeners ********************/
/****************************************************/

NGINZ_INLINE static int pp_recvmsg_helper(int through, int*target, aroop_txt_t*cmd) {
	//printf("There is new client, we need to accept it in worker process\n");
	union {
		int target;
		char buf[CMSG_SPACE(sizeof(int))];
	} intbuf;
	struct msghdr msg;
	struct iovec iov[1];
	struct cmsghdr *control_message = NULL;
	memset(&intbuf, 0, sizeof(intbuf));
	// sanity check
	if(through == -1)
		return -1;
	memset(&msg, 0, sizeof(msg));
	memset(iov, 0, sizeof(iov));

	if(aroop_txt_capacity(cmd) < 128) {
		aroop_txt_destroy(cmd);
		aroop_txt_embeded_buffer(cmd, 128);
	}
	iov[0].iov_base = aroop_txt_to_string(cmd);
	iov[0].iov_len  = aroop_txt_capacity(cmd);
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_control = intbuf.buf;
	msg.msg_controllen = CMSG_SPACE(sizeof(int));
	
	int recvlen = 0;
	if(recvlen = recvmsg(through, &msg, 0) < 0) {
		syslog(LOG_ERR, "Cannot recv msg:%s\n", strerror(errno));
		return -1;
	}
	if(msg.msg_iovlen == 1 && iov[0].iov_len > 0) {
		aroop_txt_set_length(cmd, iov[0].iov_len);
	}
	for(control_message = CMSG_FIRSTHDR(&msg);
		control_message != NULL;
		control_message = CMSG_NXTHDR(&msg,
				   control_message)) {
		if( (control_message->cmsg_level == SOL_SOCKET) &&
		(control_message->cmsg_type == SCM_RIGHTS) )
		{
			*target = *((int *) CMSG_DATA(control_message));
		}
	}
	return 0;
}

static aroop_txt_t recv_buffer;
static int on_bubble_down(int fd, int events, const void*unused) {
	//printf("There is bubble_down from the parent\n");
	aroop_txt_set_length(&recv_buffer, 1); // without it aroop_txt_to_string() will give NULL
	//char rbuf[255];
	//int count = recv(parent, rbuf, sizeof(rbuf), 0);
	aroop_assert(fd == parent);
	int count = recv(fd, aroop_txt_to_string(&recv_buffer), aroop_txt_capacity(&recv_buffer), 0);
	if(count <= 0) {
		syslog(LOG_ERR, "Error receiving bubble_down:%s\n", strerror(errno));
		return 0;
	}

	aroop_txt_set_length(&recv_buffer, count);
	//aroop_txt_embeded_rebuild_and_set_content(&recv_buffer, rbuf)
	aroop_txt_t x = {};
	//printf("There is bubble_down from the parent, %d, (count=%d)\n", (int)aroop_txt_char_at(&recv_buffer, 0), count);
	binary_unpack_string(&recv_buffer, 0, &x); // needs cleanup
	if(aroop_txt_is_empty(&x)) {
		aroop_txt_destroy(&x);
		return 0;
	}
	//printf("request from parent %s\n", aroop_txt_to_string(&x));
	aroop_txt_t input = {};
	aroop_txt_t output = {};
	pm_call(&x, &input, &output);
	aroop_txt_destroy(&input);
	aroop_txt_destroy(&output);
	aroop_txt_destroy(&x);
	return 0;
}

static int skipped = 0;
static int skip_load() {
	int load = event_loop_fd_count();
	load = load + skipped;
	skipped++;
	return (load % (NGINZ_NUMBER_OF_PROCESSORS-1));
}

static int on_bubble_down_send_socket(int fd, int events, const void*unused) {
	int port = 0;
	int destpid = 0;
	int acceptfd = -1;
	aroop_txt_t cmd = {};
	aroop_assert(fd == mparent);
	do {
		if(pp_recvmsg_helper(fd, &acceptfd, &cmd)) {
			break;
		}
		binary_coder_fixup(&cmd);
		//binary_coder_debug_dump(&cmd);
		binary_unpack_int(&cmd, 0, &destpid);
		if(destpid > 0 && destpid != getpid()) {
			// it is not ours
			//printf("It(%d) is not ours(%d) on bubble_down doing more bubble_down\n", destpid, getpid());
			if(destpid < getpid()) {
				syslog(LOG_ERR, "BUG, it cannot happen, we not not bubble_uping anymore\n");
				break;
			}
			pp_bubble_down_send_socket(acceptfd, &cmd);
			break;
		}
		if(destpid <= 0 && mchild != -1 && skip_load()) {
			//printf("balancing load on %d\n", getpid());
			pp_bubble_down_send_socket(acceptfd, &cmd);
			break;
		}
		binary_unpack_int(&cmd, 1, &port);
		struct protostack*stack = protostack_get(port);
		aroop_assert(stack != NULL);
		stack->on_connection_bubble(acceptfd, &cmd);
	} while(0);
	aroop_txt_destroy(&cmd); // cleanup
	return 0;
}

static int on_bubble_up(int fd, int events, const void*unused) {
	aroop_assert(fd == child);
	aroop_txt_set_length(&recv_buffer, 1); // without it aroop_txt_to_string() will give NULL
	int count = recv(fd, aroop_txt_to_string(&recv_buffer), aroop_txt_capacity(&recv_buffer), 0);
	// TODO do something with the recv_buffer
}

static int on_bubble_up_send_socket(int fd, int events, const void*unused) {
	int port = 0;
	int destpid = 0;
	int acceptfd = -1;
	aroop_txt_t cmd = {};
	aroop_assert(fd == mchild);
	if(is_master()) {
		aroop_assert("We cannot handle client in master\n");
	}
	do {
		if(pp_recvmsg_helper(fd, &acceptfd, &cmd)) {
			break;
		}
		binary_coder_fixup(&cmd);
		//binary_coder_debug_dump(&cmd);
		binary_unpack_int(&cmd, 0, &destpid);
		if(destpid > 0 && destpid != getpid()) {
			//printf("It(%d) is not ours(%d) on bubble_up doing bubble_down\n", destpid, getpid());
			// it is not ours
			if(destpid > getpid()) {
				syslog(LOG_ERR, "BUG, it cannot happen, we not not bubble_downing anymore\n");
				break;
			}
			pp_bubble_up_send_socket(acceptfd, &cmd);
			break;
		}
		if(destpid <= 0 && mparent != -1 && skip_load()) {
			//printf("balancing load on %d\n", getpid());
			pp_bubble_up_send_socket(acceptfd, &cmd);
			break;
		}
		binary_unpack_int(&cmd, 1, &port);
		struct protostack*stack = protostack_get(port);
		aroop_assert(stack != NULL);
		stack->on_connection_bubble(acceptfd, &cmd);
	} while(0);
	aroop_txt_destroy(&cmd);
	return 0;
}

static int pipefd[2];
static int mpipefd[2];
/****************************************************/
/********** Fork event listeners ********************/
/****************************************************/
static int pp_fork_before_callback(aroop_txt_t*input, aroop_txt_t*output) {
	//if(pipe(pipefd) || pipe(mpipefd)) {
	if(socketpair(AF_UNIX, SOCK_DGRAM, 0, pipefd) || socketpair(AF_UNIX, SOCK_DGRAM, 0, mpipefd)) {
		syslog(LOG_ERR, "Failed to create pipe:%s\n", strerror(errno));
		return -1;
	}
	return 0;
}

static int pp_fork_child_after_callback(aroop_txt_t*input, aroop_txt_t*output) {
	/****************************************************/
	/********* Cleanup old parent fds *******************/
	/****************************************************/
	event_loop_unregister_fd(parent); // we have nothing to do with old parents
	event_loop_unregister_fd(mparent); // we have nothing to do with old parents
	if(parent > 0)close(parent);
	if(mparent > 0)close(mparent);
	/****************************************************/
	/********* Register new parent fds ******************/
	/****************************************************/
	parent = pipefd[1];
	mparent = mpipefd[1];
	event_loop_register_fd(parent, on_bubble_down, NULL, NGINZ_POLL_ALL_FLAGS);
	event_loop_register_fd(mparent, on_bubble_down_send_socket, NULL, NGINZ_POLL_ALL_FLAGS);
	// cleanup child
	event_loop_unregister_fd(child);
	event_loop_unregister_fd(mchild);
	aroop_assert(child == -1);
	aroop_assert(mchild == -1);
	child = -1;
	mchild = -1;
	//close(pipefd[0]);
	//close(mpipefd[0]);
	return 0;
}

static int pp_fork_parent_after_callback(aroop_txt_t*input, aroop_txt_t*output) {
	/****************************************************/
	/********* We care about the child ******************/
	/****************************************************/
	aroop_assert(child == -1);
	aroop_assert(mchild == -1);
	child = pipefd[0];
	mchild = mpipefd[0];
	//printf("child fds %d,%d\n", pipefd[0], mpipefd[0]);
	event_loop_register_fd(child, on_bubble_up, NULL, NGINZ_POLL_ALL_FLAGS);
	event_loop_register_fd(mchild, on_bubble_up_send_socket, NULL, NGINZ_POLL_ALL_FLAGS);
	//close(pipefd[1]);
	return 0;
}

static int pp_fork_callback_desc(aroop_txt_t*plugin_space,aroop_txt_t*output) {
	return plugin_desc(output, "pipeline", "fork", plugin_space, __FILE__, "It allows the processes to pipeline messages to and forth.\n");
}

/****************************************************/
/****** Module constructors and destructors *********/
/****************************************************/
int pp_module_init() {
	aroop_txt_embeded_buffer(&recv_buffer, 255);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/before");
	pm_plug_callback(&plugin_space, pp_fork_before_callback, pp_fork_callback_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/child/after");
	pm_plug_callback(&plugin_space, pp_fork_child_after_callback, pp_fork_callback_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/parent/after");
	pm_plug_callback(&plugin_space, pp_fork_parent_after_callback, pp_fork_callback_desc);
	ping_module_init();
}

int pp_module_deinit() {
	ping_module_deinit();
	// TODO unregister all
	aroop_txt_destroy(&recv_buffer);
}

C_CAPSULE_END
