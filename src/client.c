#include "client.h"

static on_msg_recived_callback callback = NULL;
static int sock;
static pthread_t read_thr;
static bool running = false;
static struct sockaddr_in wetalk_server;

static bool client_connect_to_server() {
	static bool connected = false;

	if (connected) {
		return true;
	}

	if (connect(sock, (struct sockaddr *) &wetalk_server, sizeof(wetalk_server)) < 0) {
		return false;
	}

	connected = true;
	return true;
}

static char* client_get_result(const char *fmt, ...) {
	char *result = (char*) walloc(sizeof(char) * BUFFER_SIZE);
	va_list args;
	va_start(args, fmt);
	vsnprintf(result, BUFFER_SIZE, fmt, args);
	va_end(args);

	int r = strlen(result);
	if (write(sock, result, r) != r) {
		wfree(result);
		return NULL;
	}

	memset(result, '\0', BUFFER_SIZE);
	r = read(sock, result, BUFFER_SIZE);
	result[r] = '\0';

	return result;
}

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
	if (!client_connect_to_server()) {
		return false;
	}

	char *buffer = client_get_result("%s %d %s", CMD_LOGIN, uid, password);

	if (strncmp(buffer, CMD_SUCCESS, strlen(CMD_SUCCESS)) != 0) {
		wfree(buffer);
		return false;
	}

	wfree(buffer);

	if (pthread_create(&read_thr, NULL, read_thread, (void*) sock) < 0) {
		return false;
	}

	return true;
}

int client_register(const char *username, const char *password) {
	if (!client_connect_to_server()) {
		return false;
	}

	char *buffer = client_get_result("%s %s %s", CMD_REGISTER, username, password);
	int uid = atoi(buffer);

	if (uid <= 0) {
		wfree(buffer);
		return 0;
	}

	wfree(buffer);
	return uid;
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

