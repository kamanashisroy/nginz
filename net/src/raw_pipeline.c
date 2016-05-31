
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
#include "parallel/pipeline.h"
#include "parallel/ping.h"
#include "protostack.h"
#include "binary_coder.h"
#include "raw_pipeline.h"

C_CAPSULE_START

NGINZ_INLINE static int pp_raw_sendmsg_helper(int through, int target, aroop_txt_t*cmd) {
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

NGINZ_INLINE int pp_raw_send_socket(int destpid, int socket, aroop_txt_t*cmd) {
	int fd = pp_get_raw_fd(destpid);
	if(fd == -1) {
		syslog(LOG_ERR, "We could not find raw fd for:%d\n", destpid);
		return -1;
	}

	return pp_raw_sendmsg_helper(fd, socket, cmd);
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
	if((recvlen = recvmsg(through, &msg, 0)) < 0) {
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

static int on_raw_recv_socket(int fd, int events, const void*unused) {
	int port = 0;
	int srcpid = 0;
	int acceptfd = -1;
	aroop_txt_t cmd = {};
	do {
		//syslog(LOG_NOTICE, "[pid:%d]\treceiving from parent", getpid());
		if(pp_recvmsg_helper(fd, &acceptfd, &cmd)) {
			break;
		}
		binary_coder_fixup(&cmd);
		//binary_coder_debug_dump(&cmd);
		binary_unpack_int(&cmd, 0, &srcpid);
		binary_unpack_int(&cmd, 1, &port);
		struct protostack*stack = protostack_get(port);
		aroop_assert(stack != NULL);
		stack->on_connection_bubble(acceptfd, &cmd);
	} while(0);
	aroop_txt_destroy(&cmd); // cleanup
	return 0;
}

static int on_raw_socket_setup(int signature, void*data) {
	assert(signature == NGINZ_PIPELINE_SIGNATURE);
	int raw_fd = *((int*)data);
	event_loop_register_fd(raw_fd, on_raw_recv_socket, NULL, NGINZ_POLL_ALL_FLAGS);
	return 0;
}

static int on_raw_socket_setup_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "raw/setup", "net", plugin_space, __FILE__, "It registers event listener for raw socket.\n");
}

int pp_raw_module_init() {
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "parallel/pipeline/raw/setup");
	pm_plug_bridge(&plugin_space, on_raw_socket_setup, on_raw_socket_setup_desc);
	return 0;
}

int pp_raw_module_deinit() {
	pm_unplug_bridge(0, on_raw_socket_setup);
	return 0;
}

C_CAPSULE_END
