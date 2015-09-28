#include "client.h"

static on_msg_recived_callback callback = NULL;
static int sock;
static pthread_t read_thr;
static bool running = false;
static struct sockaddr_in wetalk_server;

static void* read_thread(void *args) {
	int bytes = 0;
	unsigned char buffer[BUFFER_SIZE];
	int sock = (int) args;

	running = true;

	while (running && (bytes = read(sock, buffer, BUFFER_SIZE - 1)) > 0) {
		buffer[bytes] = '\0';

		if (!strcmp(buffer, CMD_SERVER_CLOSED)) {
			break;
		}

		callback(buffer);
		memset(buffer, '\0', BUFFER_SIZE);
	}

	callback(NULL);
	running = false;
}

bool client_init(const char *address, on_msg_recived_callback cb) {
	if (!address || !cb) {
		return false;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		// wetalk_error("Failed to create socket\n");
		return false;
	}

	memset(&wetalk_server, 0, sizeof(wetalk_server));
	wetalk_server.sin_family = AF_INET;
	wetalk_server.sin_addr.s_addr = inet_addr(address);
	wetalk_server.sin_port = htons(LISTEN_PORT);

	callback = cb;
	return true;
}

bool client_login(int uid, const char *password) {
	if (connect(sock, (struct sockaddr *) &wetalk_server, sizeof(wetalk_server)) < 0) {
		// wetalk_error("Failed to connect with server\n");
		return false;
	}

	int r = strlen(CMD_LOGIN);
	if (write(sock, CMD_LOGIN, r) != r) {
		// wetalk_error("Failed to send login command\n");
		return false;
	}

	if (pthread_create(&read_thr, NULL, read_thread, (void*) sock) < 0) {
		// wetalk_error("Failed to create thread\n");
		return false;
	}

	return true;
}

void client_logout() {
	int r = strlen(CMD_EXIT);
	if (write(sock, CMD_EXIT, r) != r) {
		wetalk_warning("Unexpected exit command has sent\n");
	}
	close(sock);
}

void client_send(const char *text) {
	int r = strlen(text);
	if (write(sock, text, r) != r) {
		wetalk_warning("Unexpected message has sent\n");
	}
}

