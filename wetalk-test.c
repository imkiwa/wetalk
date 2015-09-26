#include "wetalk.h"
#include "server.h"

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
	size_t text_length;

	if (argc != 2) {
		fprintf(stderr, "usage: wetalk-test <ip> <string-to-send>\n");
		return 1;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		die("Failed to create socket");
	}

	memset(&wetalk_server, 0, sizeof(wetalk_server));      
	wetalk_server.sin_family = AF_INET;                
	wetalk_server.sin_addr.s_addr = inet_addr(argv[1]);
	wetalk_server.sin_port = htons(LISTEN_PORT);     

	if (connect(sock, (struct sockaddr *) &wetalk_server, sizeof(wetalk_server)) < 0) {
		die("Failed to connect with server");
	}

	write(sock, CMD_LOGIN, strlen(CMD_LOGIN));
	
	int bytes = 0;
	char input[BUFFER_SIZE];

	while((bytes = read(sock, buffer, BUFFER_SIZE - 1)) > 0) {
		memset(input, '\0', BUFFER_SIZE);

		buffer[bytes] = '\0';
		fprintf(stderr, "[Client] Recive: %s\n", buffer);

		if (!strcmp(buffer, CMD_SERVER_CLOSED)) {
			break;
		}

		if ((text_length = read(fileno(stdin), input, BUFFER_SIZE - 1)) < 0) {
			break;
		}

		if (input[text_length] == '\n') {
			input[text_length] = '\0';
		}

		write(sock, input, text_length);
	}

	write(sock, CMD_EXIT, strlen(CMD_EXIT));
	close(sock);
	return 0;
}
