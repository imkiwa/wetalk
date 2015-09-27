#include "wetalk.h"
#include "user.h"
#include "server.h"
#include <signal.h>

static void on_signal(int sig) {
	if (sig == SIGINT) {
		server_stop();

		printf("\n");
		exit(0);
	}
}

static void on_client(client_info *info) {
}

int main(int argc, char **argv) {
	signal(SIGINT, on_signal);

	if (!pid_init()) {
		wetalk_error("Server already running.");
	}

	if (!server_init(on_client)) {
		perror("server_init");
		return 1;
	}
	
	server_loop();
	return 0;
}

