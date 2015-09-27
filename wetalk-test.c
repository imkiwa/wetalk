#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <readline/readline.h>

#include "include/server_cmd.h"

#ifndef BUFFER_SIZE
#	define BUFFER_SIZE 1024
#	define READ_FMT "%1023s"
#endif

void die(char *mess)
{
	perror(mess);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in wetalk_server;
	unsigned char buffer[BUFFER_SIZE];

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		die("Failed to create socket");
	}

	memset(&wetalk_server, 0, sizeof(wetalk_server));      
	wetalk_server.sin_family = AF_INET;                
	wetalk_server.sin_addr.s_addr = inet_addr("127.0.0.1");
	wetalk_server.sin_port = htons(LISTEN_PORT);     

	if (connect(sock, (struct sockaddr *) &wetalk_server, sizeof(wetalk_server)) < 0) {
		die("Failed to connect with server");
	}

	write(sock, CMD_LOGIN, strlen(CMD_LOGIN));
	
	int bytes = 0;
	char *input = NULL;

	while((bytes = read(sock, buffer, BUFFER_SIZE - 1)) > 0) {
		if (input) {
			free(input);
			input = NULL;
		}

		buffer[bytes] = '\0';
		fprintf(stderr, "[Client] Recive: %s\n", buffer);

		if (!strcmp(buffer, CMD_SERVER_CLOSED)) {
			break;
		}

		input = readline("Wetalk> ");
		if (input && *input) {
			add_history(input);
			write(sock, input, strlen(input));
		} else {
			break;
		}
	}

	write(sock, CMD_EXIT, strlen(CMD_EXIT));
	close(sock);
	return 0;
}
