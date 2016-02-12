
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
#include "plugin_manager.h"
#include "event_loop.h"
#include "nginz_config.h"
#include "net/tcp_listener.h"

C_CAPSULE_START

static int tcp_sock = -1;
static int on_connection(int status, const void*unused) {
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(struct sockaddr_in);
	int client_fd = accept(tcp_sock, (struct sockaddr *) &client_addr, &client_addr_len);
	if(client_fd < 0) {
		perror("Accept failed\n");
		event_loop_unregister_fd(tcp_sock);
		close(tcp_sock);
		close(client_fd);
		return -1;
	}
	aroop_txt_t bin = {};
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset_for_pid(&bin, 0);
	aroop_txt_t welcome_command = {};
	aroop_txt_embeded_set_static_string(&welcome_command, "chat/welcome"); 
	binary_pack_string(&bin, &welcome_command);
	pp_pingmsg(client_fd, &bin);
	return 0;
}

static int tcp_listener_stop(aroop_txt_t*input, aroop_txt_t*output) {
	event_loop_unregister_fd(tcp_sock);
	if(tcp_sock > 0)close(tcp_sock);
	tcp_sock = -1;
}

static int tcp_listener_stop_desc(aroop_txt_t*plugin_space,aroop_txt_t*output) {
	return plugin_desc(output, "tcp_listener", "fork", plugin_space, __FILE__, "It stops tcp_listener in child process\n");
}


int tcp_listener_init() {
	if(!is_master()) // we only start it in the master
		return 0;
	aroop_assert(tcp_sock == -1);
	if((tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("Failed to create socket");
		return -1;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	inet_aton("0.0.0.0", &(addr.sin_addr));
	addr.sin_port = htons(NGINZ_DEFAULT_PORT);
	int sock_flag = 1;
	setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&sock_flag, sizeof(sock_flag));
	if(bind(tcp_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("Failed to bind");
		close(tcp_sock);
		return -1;
	}
#define DEFAULT_SYN_BACKLOG 1024 /* XXX we are setting this too high */
	if(listen(tcp_sock, DEFAULT_SYN_BACKLOG) < 0) {
		perror("Failed to listen");
		close(tcp_sock);
		return -1;
	}
	printf("listening ...\n");
	event_loop_register_fd(tcp_sock, on_connection, NULL, POLLIN | POLLPRI | POLLHUP);
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/child/after");
	pm_plug_callback(&plugin_space, tcp_listener_stop, tcp_listener_stop_desc);
	aroop_txt_embeded_set_static_string(&plugin_space, "shake/softquitall");
	pm_plug_callback(&plugin_space, tcp_listener_stop, tcp_listener_stop_desc);
	return 0;
}

int tcp_listener_deinit() {
	event_loop_unregister_fd(tcp_sock);
	if(tcp_sock > 0)close(tcp_sock);
	tcp_sock = -1;
	pm_unplug_callback(0, tcp_listener_stop);
	pm_unplug_callback(0, tcp_listener_stop);
	return 0;
}

C_CAPSULE_END
