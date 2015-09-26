#include "wetalk.h"
#include "server.h"

#define BUFFSIZE 1024

void die(char *mess)
{
	perror(mess);
	exit(1);
}

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in wetalk_server;
	unsigned char buffer[BUFFSIZE];
	size_t text_length;

	if (argc != 3) {
		fprintf(stderr, "usage: wetalk-test <ip> <string-to-send>\n");
		return 1;
	}

	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		die("Failed to create socket");
	}

	memset(&wetalk_server, 0, sizeof(wetalk_server));      
	wetalk_server.sin_family = AF_INET;                
	wetalk_server.sin_addr.s_addr = inet_addr(argv[1]);
	wetalk_server.sin_port = htons(LISTEN_PORT);     

	if (connect(sock, (struct sockaddr *) &wetalk_server, sizeof(wetalk_server)) < 0) {
		die("Failed to connect with server");
	}

	char *text_to_send = argv[2];
	text_length = strlen(text_to_send);
	if (write(sock, text_to_send, text_length) != text_length) {
		die("Mismatch in number of sent bytes");
	}

	
	int bytes = read(sock, buffer, BUFFSIZE - 1);
	buffer[bytes] = '\0';
	fprintf(stderr, "[Client] Recive: %s\n", buffer);

	write(sock, CMD_EXIT, strlen(CMD_EXIT));
	close(sock);
	return 0;
}
