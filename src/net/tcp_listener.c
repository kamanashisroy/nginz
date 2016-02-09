
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
#include "net/tcp_listener.h"

C_CAPSULE_START

static int on_connection(int status) {
	printf("New incoming connection\n");
}

int tcp_listener_init() {
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	inet_aton("0.0.0.0", &(addr.sin_addr));
	addr.sin_port = htons(82);
	char sock_flag = 0;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&sock_flag, sizeof sock_flag);
	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("Failed to bind at %s\n", strerror(errno));
		close(sock);
		return -1;
	}
#define DEFAULT_SYN_BACKLOG 1024 /* XXX we are setting this too high */
	if(listen(sock, DEFAULT_SYN_BACKLOG) < 0) {
		printf("Failed to listen on %s\n", strerror(errno));
		close(sock);
		return -1;
	}
	event_loop_register_fd(sock, on_connection, POLLIN | POLLPRI | POLLHUP);
}

int tcp_listener_deinit() {
}

C_CAPSULE_END
