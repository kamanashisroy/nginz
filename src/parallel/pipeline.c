
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

NGINZ_INLINE int pp_ping(aroop_txt_t*pkt) {
	// sanity check
	if(child == -1)
		return -1;
	send(child, aroop_txt_to_string(pkt), aroop_txt_length(pkt), 0);
	return 0;
}

NGINZ_INLINE static int pp_sendmsg_helper(int through, int target) {
	union {
		int target;
		char buf[CMSG_SPACE(sizeof(int))];
	} intbuf;
	intbuf.target = target;
	struct msghdr msg;
	struct iovec iov[1];
	struct cmsghdr *control_message = NULL;
	memset(&intbuf, 0, sizeof(intbuf));
	char data[1];
	// sanity check
	if(through == -1)
		return -1;
	memset(&msg, 0, sizeof(msg));
	memset(iov, 0, sizeof(iov));

	iov[0].iov_base = data;
	iov[0].iov_len  = 1;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_control = intbuf.buf;
	msg.msg_controllen = CMSG_SPACE(sizeof(int));
	control_message = CMSG_FIRSTHDR(&msg);
	control_message->cmsg_level = SOL_SOCKET;
	control_message->cmsg_type = SCM_RIGHTS;
	control_message->cmsg_len = CMSG_LEN(sizeof(int));
	*((int *) CMSG_DATA(control_message)) = target;
	printf("Sending fd %d to worker\n", target);
	
	if(sendmsg(through, &msg, 0) < 0) {
		perror("Cannot send msg");
		return -1;
	}
	return 0;
}

NGINZ_INLINE int pp_pingmsg(int socket) {
	printf("Sending fd %d to worker\n", socket);
	return pp_sendmsg_helper(mchild, socket);
}

NGINZ_INLINE int pp_pong(aroop_txt_t*pkt) {
	// sanity check
	if(parent == -1)
		return -1;
	send(parent, aroop_txt_to_string(pkt), aroop_txt_length(pkt), 0);
	return 0;
}

NGINZ_INLINE int pp_pongmsg(int socket) {
	return pp_sendmsg_helper(mparent, socket);
}

NGINZ_INLINE int is_master() {
	return (parent == -1);
}

/****************************************************/
/********** Pipe event listeners ********************/
/****************************************************/

NGINZ_INLINE static int pp_recvmsg_helper(int through, int*target) {
	printf("There is new client, we need to accept it in worker process\n");
	union {
		int target;
		char buf[CMSG_SPACE(sizeof(int))];
	} intbuf;
	struct msghdr msg;
	struct iovec iov[1];
	struct cmsghdr *control_message = NULL;
	memset(&intbuf, 0, sizeof(intbuf));
	char data[1];
	// sanity check
	if(through == -1)
		return -1;
	memset(&msg, 0, sizeof(msg));
	memset(iov, 0, sizeof(iov));

	iov[0].iov_base = data;
	iov[0].iov_len  = 1;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_control = intbuf.buf;
	msg.msg_controllen = CMSG_SPACE(sizeof(int));
	
	if(recvmsg(through, &msg, 0) < 0) {
		perror("Cannot recv msg");
		return -1;
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
static int on_ping(int events, const void*unused) {
	printf("There is ping from the parent\n");
	aroop_txt_set_length(&recv_buffer, 1); // without it aroop_txt_to_string() will give NULL
	//char rbuf[255];
	//int count = recv(parent, rbuf, sizeof(rbuf), 0);
	int count = recv(parent, aroop_txt_to_string(&recv_buffer), aroop_txt_capacity(&recv_buffer), 0);
	if(count <= 0) {
		perror("Error receiving ping");
		return 0;
	}

	aroop_txt_set_length(&recv_buffer, count);
	//aroop_txt_embeded_rebuild_and_set_content(&recv_buffer, rbuf)
	aroop_txt_t x = {};
	printf("There is ping from the parent, %d, (count=%d)\n", (int)aroop_txt_char_at(&recv_buffer, 0), count);
	binary_unpack_string(&recv_buffer, 0, &x);
	if(aroop_txt_length(&x)) {
		printf("request from parent %s\n", aroop_txt_to_string(&x));
		aroop_txt_t input = {};
		aroop_txt_t output = {};
		pm_call(&x, &input, &output);
		//aroop_txt_destroy(&input);
		aroop_txt_destroy(&output);
	}
}

static int on_pingmsg(int events, const void*unused) {
	int acceptfd = -1;
	if(pp_recvmsg_helper(mparent, &acceptfd)) {
		return -1;
	}
	struct protostack*stack = protostack_get(NGINZ_DEFAULT_PORT);
	stack->on_connect(acceptfd);
	return 0;
}

static int on_pong(int events, const void*unused) {
	int count = recv(child, aroop_txt_to_string(&recv_buffer), aroop_txt_capacity(&recv_buffer), 0);
	// TODO do something with the recv_buffer
}

static int on_pongmsg(int events, const void*unused) {
	int acceptfd = -1;
	if(pp_recvmsg_helper(mparent, &acceptfd)) {
		return -1;
	}
	if(acceptfd != -1)
		send(acceptfd, "Hi again\n", 3, 0);
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
		perror("Faild to create pipe");
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
	event_loop_register_fd(parent, on_ping, NULL, NGINZ_POLL_ALL_FLAGS);
	event_loop_register_fd(mparent, on_pingmsg, NULL, NGINZ_POLL_ALL_FLAGS);
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
	printf("child fds %d,%d\n", pipefd[0], mpipefd[0]);
	event_loop_register_fd(child, on_pong, NULL, NGINZ_POLL_ALL_FLAGS);
	event_loop_register_fd(mchild, on_pongmsg, NULL, NGINZ_POLL_ALL_FLAGS);
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
