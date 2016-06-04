
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_any_obj.h>
#include <aroop/opp/opp_str2.h>
#include <aroop/aroop_memory_profiler.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "plugin.h"
#include "log.h"
#include "plugin_manager.h"
#include "event_loop.h"
#include "nginz_config.h"
#include "protostack.h"
#include "parallel/pipeline.h"
#include "tcp_listener_internal.h"

C_CAPSULE_START

static int tcp_ports[NGINZ_MAX_PROTO];
static int tcp_sockets[NGINZ_MAX_PROTO] = { -1, -1, -1, -1};

static int on_connection(int fd, int status, const void*pstack) {
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(struct sockaddr_in);
	int client_fd = accept(fd, (struct sockaddr *) &client_addr, &client_addr_len);
	if(client_fd < 0) {
		syslog(LOG_ERR, "Accept failed:%s\n", strerror(errno));
		event_loop_unregister_fd(fd);
		close(fd);
		close(client_fd);
		return -1;
	}
	struct protostack*stack = (struct protostack*)pstack;
	aroop_assert(stack != NULL);
	stack->on_tcp_connection(client_fd);
	return 0;
}

static int tcp_listener_stop(aroop_txt_t*input, aroop_txt_t*output) {
	int i = 0;
	int count = sizeof(tcp_sockets)/sizeof(*tcp_sockets);
	for(i = 0; i < count && tcp_sockets[i] != -1; i++) {
		event_loop_unregister_fd(tcp_sockets[i]);
		if(tcp_sockets[i] > 0)close(tcp_sockets[i]);
		tcp_sockets[i] = -1;
	}
	return 0;
}

static int tcp_listener_stop_desc(aroop_txt_t*plugin_space,aroop_txt_t*output) {
	return plugin_desc(output, "tcp_listener", "fork", plugin_space, __FILE__, "It stops tcp_listener in child process\n");
}

static int tcp_listener_start() {
	int i = 0;
	int count = sizeof(tcp_sockets)/sizeof(*tcp_sockets);
	aroop_assert(tcp_ports[0]);
	for(i = 0; i < count && tcp_ports[i] != 0; i++) {
		aroop_assert(tcp_sockets[i] == -1);
		if((tcp_sockets[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			syslog(LOG_ERR, "Failed to create socket:%s\n", strerror(errno));
			continue;
		}
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		inet_aton("0.0.0.0", &(addr.sin_addr));
		addr.sin_port = htons(tcp_ports[i]);
		int sock_flag = 1;
		setsockopt(tcp_sockets[i], SOL_SOCKET, SO_REUSEADDR, (char*)&sock_flag, sizeof(sock_flag));
		if(bind(tcp_sockets[i], (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			syslog(LOG_ERR, "tcp_listener,c Failed to bind port %d:%s\n", tcp_ports[i], strerror(errno));
			close(tcp_sockets[i]);
			continue;
		}
		if(listen(tcp_sockets[i], NGINZ_TCP_LISTENER_BACKLOG) < 0) {
			syslog(LOG_ERR, "Failed to listen:%s\n", strerror(errno));
			close(tcp_sockets[i]);
			continue;
		}
		syslog(LOG_INFO, "tcp_listener.c: listening to port %d\n", tcp_ports[i]);
		struct protostack*stack = protostack_get(tcp_ports[i]);
		aroop_assert(stack != NULL);
		event_loop_register_fd(tcp_sockets[i], on_connection, stack, NGINZ_POLL_LISTEN_FLAGS);

	}
	return 0;
}

int tcp_listener_init() {
	if(!is_master()) // we only start it in the master
		return 0;
	memset(tcp_ports, 0, sizeof(tcp_ports));
	protostack_get_ports_internal(tcp_ports);
	tcp_listener_start();
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/child/after");
	pm_plug_callback(&plugin_space, tcp_listener_stop, tcp_listener_stop_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/softquitall");
	pm_plug_callback(&plugin_space, tcp_listener_stop, tcp_listener_stop_desc);
	return 0;
}

int tcp_listener_deinit() {
	tcp_listener_stop(NULL, NULL);
	pm_unplug_callback(0, tcp_listener_stop);
	pm_unplug_callback(0, tcp_listener_stop);
	return 0;
}

C_CAPSULE_END
