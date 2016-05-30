
#include "aroop/aroop_core.h"
#include "aroop/core/thread.h"
#include "aroop/core/xtring.h"
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "aroop/opp/opp_any_obj.h"
#include "aroop/opp/opp_str2.h"
#include "aroop/aroop_memory_profiler.h"
#include "nginz_config.h"
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "event_loop.h"
#include "shake/test.h"
#include <sys/socket.h>
#include <sys/un.h>
#include "shake.h"
#include "shake/shake_internal.h"
#include "parallel/pipeline.h"
#include "scanner.h"

C_CAPSULE_START


#define SOCK_FILE "/tmp/nginz.sock"
static int internal_unix_socket = -1;

static int on_shake_connection_helper(int fd) {
	// read data from stdin
	char cmd[128]; 
	int cmdlen = read(fd, cmd, sizeof(cmd)); // XXX it will block the total process ..
	if(cmdlen <= 0)
		return 0;
	aroop_txt_t xcmd;
	aroop_txt_embeded_set_content(&xcmd, cmd, cmdlen, NULL);
	aroop_txt_t target = {};
	aroop_txt_t input = {};
	aroop_txt_t output = {};
	shotodol_scanner_next_token(&xcmd, &target);
	do {
		if(aroop_txt_length(&target) == 0)
			break;
		aroop_txt_t plugin_space = {};
		aroop_txt_embeded_stackbuffer(&plugin_space, 64);
		aroop_txt_concat_string(&plugin_space, "shake/");
		aroop_txt_concat(&plugin_space, &target);
		aroop_txt_embeded_txt_copy_shallow(&input, &xcmd); // pass the command argument as input
		aroop_txt_shift(&input, 1); // skip the white space
		pm_call(&plugin_space, &input, &output);
		aroop_txt_zero_terminate(&output);
		//printf("%s", aroop_txt_to_string(&output));
		write(fd, aroop_txt_to_string(&output), aroop_txt_length(&output));
	} while(0);
	// cleanup 
	aroop_txt_destroy(&input);
	aroop_txt_destroy(&output);
	aroop_txt_destroy(&target);
	return 0;
}

static int on_shake_connection(int fd, int status, const void*unused) {
	if(internal_unix_socket == -1)
		return 0;
	aroop_assert(internal_unix_socket == fd);
	int client_fd = accept(internal_unix_socket, NULL, NULL);
	if(client_fd < 0) {
		syslog(LOG_ERR, "Shake: Accept failed:%s\n", strerror(errno));
		close(client_fd);
		return -1;
	}
	on_shake_connection_helper(client_fd);
	close(client_fd);
	return 0;
}

static int shake_listen_on_unix_socket() {
	aroop_assert(internal_unix_socket == -1);
	if((internal_unix_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		syslog(LOG_ERR, "Shake: failed to create unix socket:%s\n", strerror(errno));
		return -1;
	}
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SOCK_FILE, sizeof(addr.sun_path)-1);
	if(bind(internal_unix_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		syslog(LOG_ERR, "shake:Failed bind:%s\n", strerror(errno));
		close(internal_unix_socket);
		return -1;
	}
	if(listen(internal_unix_socket, 5) < 0) {
		syslog(LOG_ERR, "shake:Failed to listen:%s\n", strerror(errno));
		close(internal_unix_socket);
		return -1;
	}
	syslog(LOG_INFO, "listening to unix socket...\n");
	//event_loop_register_fd(STDIN_FILENO, on_shake_command, NULL, NGINZ_POLL_ALL_FLAGS);
	event_loop_register_fd(internal_unix_socket, on_shake_connection, NULL, NGINZ_POLL_ALL_FLAGS);
	return 0;
}

static int shake_stop_on_fork(aroop_txt_t*input, aroop_txt_t*output) {
	//close(stdin);
	event_loop_unregister_fd(internal_unix_socket);
	close(internal_unix_socket);
	internal_unix_socket = -1;
	return 0;
}

static int shake_stop_on_fork_desc(aroop_txt_t*plugin_space,aroop_txt_t*output) {
	return plugin_desc(output, "shake", "fork", plugin_space, __FILE__, "It stops control socket in child process\n");
}

int shake_module_init() {
	if(!is_master()) {
		return 0;
	}
	unlink(SOCK_FILE);
	aroop_txt_t plugin_space = {};
	// register shake shell
	shake_listen_on_unix_socket();
	test_module_init();
	help_module_init();
	enumerate_module_init();
	aroop_txt_embeded_set_static_string(&plugin_space, "fork/child/after");
	pm_plug_callback(&plugin_space, shake_stop_on_fork, shake_stop_on_fork_desc);
	return 0;
}

int shake_module_deinit() {
	if(!is_master()) {
		return 0;
	}
	//event_loop_unregister_fd(STDIN_FILENO);
	event_loop_unregister_fd(internal_unix_socket);
	if(internal_unix_socket != -1) {
		close(internal_unix_socket);
		internal_unix_socket = -1;
	}
	unlink(SOCK_FILE);
	help_module_deinit();
	test_module_deinit();
	enumerate_module_deinit();
	return 0;
}

C_CAPSULE_END

