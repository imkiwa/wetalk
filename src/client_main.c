#include "wetalk.h"
#include "user.h"
#include "server.h"

#include <readline/readline.h>

static bool client_main_run = true;

void* read_thread(void *args) {
	int bytes = 0;
	unsigned char buffer[BUFFER_SIZE];
	int sock = (int) args;

	while (client_main_run && (bytes = read(sock, buffer, BUFFER_SIZE - 1)) > 0) {
		buffer[bytes] = '\0';
		if (!strcmp(buffer, CMD_SERVER_CLOSED)) {
			break;
		}

		fprintf(stderr, "\n[!NewMessage!] %s\n", buffer);
		memset(buffer, '\0', BUFFER_SIZE);
	}

	client_main_run = false;
}



int client_main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "You must give me a ip address.\n");
		return wetalk_usage("wetalk");
	}

	int sock;
	struct sockaddr_in wetalk_server;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		wetalk_error("Failed to create socket\n");
	}

	memset(&wetalk_server, 0, sizeof(wetalk_server));      
	wetalk_server.sin_family = AF_INET;                
	wetalk_server.sin_addr.s_addr = inet_addr(argv[1]);
	wetalk_server.sin_port = htons(LISTEN_PORT);     

	if (connect(sock, (struct sockaddr *) &wetalk_server, sizeof(wetalk_server)) < 0) {
		wetalk_error("Failed to connect with server\n");
	}

	pthread_t readthr;
	if (pthread_create(&readthr, NULL, read_thread, (void*) sock) < 0) {
		wetalk_error("Failed to create thread\n");
	}

	int r = strlen(CMD_LOGIN);
	if (write(sock, CMD_LOGIN, r) != r) {
		wetalk_error("Failed to send login command\n");
	}
	
	char *input = NULL;
	while(client_main_run && (input = readline("")) != NULL) {
		if (*input) {
			add_history(input);
			r = strlen(input);
			if (write(sock, input, r) != r) {
				wetalk_warning("Unexpected message has sent\n");
			}
		}
		if (input) {
			free(input);
			input = NULL;
		}
	}

	client_main_run = false;

	r = strlen(CMD_EXIT);
	if (write(sock, CMD_EXIT, r) != r) {
		wetalk_warning("Unexpected exit command has sent\n");
	}
	close(sock);
	return 0;
}
