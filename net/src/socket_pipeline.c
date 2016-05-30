
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

#if 0
static int skipped = 0;
static int skip_load() {
	int load = event_loop_fd_count();
	load = load + skipped;
	skipped++;
	return (load % (NGINZ_NUMBER_OF_PROCESSORS-1));
}
#endif
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
	//syslog(LOG_NOTICE, "[pid:%d]\tsending fd %d to child", getpid(), socket);
	return pp_sendmsg_helper(mchild, socket, cmd);
}


NGINZ_INLINE int pp_bubble_up_send_socket(int socket, aroop_txt_t*cmd) {
	if(mparent == -1) {
		//syslog(LOG_NOTICE, "[pid:%d]\tsending fd %d to last child", getpid(), socket);
		return pp_sendmsg_helper(mloop_pipefd[1], socket, cmd);
	}
	//syslog(LOG_NOTICE, "[pid:%d]\tsending fd %d to parent", getpid(), socket);
	return pp_sendmsg_helper(mparent, socket, cmd);
}

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

static int on_bubble_down_recv_socket(int fd, int events, const void*unused) {
	int port = 0;
	int destpid = 0;
	int acceptfd = -1;
	aroop_txt_t cmd = {};
	aroop_assert(fd == mparent);
	do {
		//syslog(LOG_NOTICE, "[pid:%d]\treceiving from parent", getpid());
		if(pp_recvmsg_helper(fd, &acceptfd, &cmd)) {
			break;
		}
		binary_coder_fixup(&cmd);
		//binary_coder_debug_dump(&cmd);
		binary_unpack_int(&cmd, 0, &destpid);
		if(destpid > 0 && destpid != getpid()) {
			// it is not ours
			if(destpid < getpid()) {
				syslog(LOG_ERR, "BUG, it cannot happen, we not not bubble down anymore\n");
				break;
			}
			pp_bubble_down_send_socket(acceptfd, &cmd);
			break;
		}
		if(is_last_worker && destpid > getpid()) {
			syslog(LOG_ERR, "BUG Invalid pid %d\n", destpid);
		}
		if(destpid <= 0 && !is_last_worker && skip_load()) {
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
		//syslog(LOG_NOTICE, "[pid:%d]\treceiving from child", getpid());
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
				syslog(LOG_ERR, "BUG, it cannot happen, we do not bubble_downing anymore\n");
				break;
			}
			pp_bubble_up_send_socket(acceptfd, &cmd);
			break;
		}
		if(destpid <= 0 && !is_first_worker && skip_load()) {
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

int spipe_module_init() {
}

int spipe_module_deinit() {
}


