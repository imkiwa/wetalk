#include "wetalk.h"
#include "server.h"
#include "walloc.h"

static client_handler client_hdl = NULL;
static bool running = false;

static int sock_fd;
static struct sockaddr_in sock_addr;

static client_info *all_client[CLIENT_MAX];

static void server_log(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	fprintf(stderr, "[Server] ");
	vfprintf(stderr, fmt, args);
	va_end(args);
}

static void server_online_add(client_info *info) {
	server_log("Adding client #%d to online list.\n", info->client_fd);
	all_client[info->client_fd] = info;
}

static void server_online_remove(client_info *info) {
	server_log("Removing client #%d from online list.\n", info->client_fd);
	all_client[info->client_fd] = NULL;
}

void server_online_foreach(online_foreach_callback fn, void *data) {
	int i = 0;
	for (i = 0; i < CLIENT_MAX; ++i) {
		fn(all_client[i], data);
	}
}

void server_sned_to_client(client_info *info, const char *msg) {
	if (!msg) {
		return;
	}

	int length = strlen(msg);
	if (write(info->client_fd, msg, length) != length) {
		server_log("Unexpected message has sent to client #%d\n", info->client_fd);
	}
}

static void* server_client_handler_caller(void *args) {
	client_info *info = (client_info*) args;
	unsigned char *buffer = walloc(sizeof(unsigned char) * BUFFER_SIZE);
	bool handled = false;

	server_log("Handler caller #%d started.\n", info->client_fd);
	server_online_add(info);

	int length = 0;
	while (running && (length = read(info->client_fd, buffer, BUFFER_SIZE - 1)) > 0) {
		buffer[length] = '\0';
		handled = false;

		// server_log("Recive from client #%d: %s\n", info->client_fd, buffer);

		if (!strcmp(buffer, CMD_EXIT)) {
			handled = true;
			break;
		} else if (!strcmp(buffer, CMD_LOGIN)) {
			handled = true;
		}

		if (!handled) {
			info->client_msg = buffer;
			client_hdl(info);
		}

		memset(buffer, '\0', BUFFER_SIZE);
	}

	server_sned_to_client(info, CMD_SERVER_CLOSED);
	server_log("Client #%d has disconnected.\n", info->client_fd);

	server_online_remove(info);
	close(info->client_fd);
	wfree(info);
	wfree(buffer);
	pthread_exit(NULL);
}

bool server_isrunning() {
	return running;
}

bool server_init(client_handler handler) {
	if (running) {
		return false;
	}

	if (!handler) {
		return false;
	}


	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		return false;
	}

	bzero(&sock_addr, sizeof(struct sockaddr_in));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(LISTEN_PORT);
	sock_addr.sin_addr.s_addr = INADDR_ANY; /* inet_addr("127.0.0.1"); */

	int r = 0;

	r = bind(sock_fd, (struct sockaddr*) &sock_addr, sizeof(struct sockaddr));
	if (r == -1) {
		return false;
	}

	r = listen(sock_fd, 3);
	if (r == -1) {
		return false;
	}

	client_hdl = handler;
	running = true;

	return true;
}

void server_loop() {
	client_info *client = NULL;

	server_log("Server is waiting for connection.\n");
	while (running) {
		client = walloc(sizeof(client_info));
		bzero(&(client->client_addr), sizeof(struct sockaddr_in));
		client->client_addr_size = sizeof(struct sockaddr_in);

		client->client_fd = accept(sock_fd, (struct sockaddr*) &(client->client_addr), &(client->client_addr_size));
		if (client->client_fd < 0) {
			wfree(client);
			continue;
		}

		server_log("New client from %s, fd #%d\n", inet_ntoa(client->client_addr.sin_addr), client->client_fd);

		if (pthread_create(&(client->client_thread), NULL, server_client_handler_caller, (void*)client) != 0) {
			close(client->client_fd);
			wfree(client);
			continue;
		}
	}

	close(sock_fd);
}

void server_stop() {
	running = false;
}

