#include "wetalk.h"
#include "user.h"
#include "server.h"

static void main_server_on_signal(int sig) {
	if (sig == SIGINT) {
		server_stop();

		printf("\n");
		exit(0);
	}
}

static void main_server_foreach_callback(client_info *info, void *data) {
	if (info) {
		server_sned_to_client(info, (char*)data);
	}
}

static void main_server_on_client(client_info *info) {
	server_online_foreach(main_server_foreach_callback, (void*)info->client_msg);
}

int server_main() {
	signal(SIGINT, main_server_on_signal);

	if (!pid_init()) {
		wetalk_error("Server already running.\n");
	}

	if (!server_init(main_server_on_client)) {
		perror("server_init");
		return 1;
	}
	
	server_loop();
	return 0;
}

