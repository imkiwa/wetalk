#include "wetalk.h"
#include "user.h"
#include "server.h"
#include "walloc.h"

static void main_server_on_signal(int sig) {
	if (sig == SIGINT) {
		server_stop();

		printf("\n");
		exit(0);
	}
}

static void main_server_foreach_callback(client_info *info, void *data) {
	if (info) {
		client_info *from = (client_info*)data;

		char *buffer = (char*) walloc(sizeof(char) * (USERNAME_MAX + strlen(from->client_msg) + 5));
		sprintf(buffer, "[%s(%d)] %s", from->client_user->username, from->client_user->uid, from->client_msg);
		
		server_sned_to_client(info, buffer);
		wfree(buffer);
	}
}

static void main_server_on_newmsg(client_info *info) {
	server_online_foreach(main_server_foreach_callback, (void*)info);
}

int server_main() {
	signal(SIGINT, main_server_on_signal);

	if (!pid_init()) {
		wetalk_error("Server already running.\n");
	}

	if (!user_init()) {
		wetalk_error("Could not load user database.\n");
	}

	if (!server_init(main_server_on_newmsg)) {
		perror("server_init");
		return 1;
	}
	
	server_loop();
	return 0;
}

