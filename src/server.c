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


static user_info* server_exec_login(const char *buffer) {
	user_info *info = NULL;
	char *password = (char*) walloc(sizeof(char) * PASSWORD_MAX);
	int uid;

	sscanf(buffer, "%*s %d %s", &uid, password);
	info = user_login(uid, password);
	wfree(password);
	return info;
}

static int server_exec_register(const char *buffer) {
	user_info *info = NULL;
	char *password = (char*) walloc(sizeof(char) * PASSWORD_MAX);
	char *username = (char*) walloc(sizeof(char) * USERNAME_MAX);

	sscanf(buffer, "%*s %s %s", username, password);
	info = user_create(username, password);
	wfree(password);
	wfree(username);

	int uid = 0;
	if (info) {
		uid = info->uid;
		wfree(info);
	}

	return uid;
}


static void server_notify_new_online_callback(client_info *info, void *data) {
	if (info) {
		client_info *from = (client_info*) data;

		char buffer[128];
		snprintf(buffer, 128, "用户 %s 已上线", from->client_user->username);
		server_sned_to_client(info, buffer);
	}
}

static void server_notify_disconn_callback(client_info *info, void *data) {
	if (info) {
		client_info *from = (client_info*) data;

		char buffer[128];
		snprintf(buffer, 128, "用户 %s 已下线", from->client_user->username);
		server_sned_to_client(info, buffer);
	}
}

static void server_notify_new_online(client_info *info) {
	server_online_foreach(server_notify_new_online_callback, (void*) info);
}

static void server_notify_disconn(client_info *info) {
	server_online_foreach(server_notify_disconn_callback, (void*) info);
}

static void* server_client_handler_caller(void *args) {
	client_info *info = (client_info*) args;
	unsigned char *buffer = walloc(sizeof(unsigned char) * BUFFER_SIZE);
	bool handled = false;
	bool longind = false;
	char *from_addr = inet_ntoa(info->client_addr.sin_addr);

	server_log("New client from %s, fd #%d\n", from_addr, info->client_fd);

	int length = 0;
	while (running && (length = read(info->client_fd, buffer, BUFFER_SIZE - 1)) > 0) {
		buffer[length] = '\0';
		handled = false;

		if (!strcmp(buffer, CMD_EXIT)) {
			handled = true;
			break;
		} else if (!strncmp(buffer, CMD_LOGIN, strlen(CMD_LOGIN))) {
			user_info *user = server_exec_login(buffer);
			if (user) {
				longind = true;
				info->client_user = user;
				server_sned_to_client(info, CMD_SUCCESS);
				server_notify_new_online(info);
				server_online_add(info);
			} else {
				server_sned_to_client(info, CMD_FAILED);
				break;
			}
			handled = true;
		} else if (!strncmp(buffer, CMD_REGISTER, strlen(CMD_REGISTER))) {
			int uid = server_exec_register(buffer);
			if (uid <= 0) {
				server_sned_to_client(info, "0");
			} else {
				char uidstring[4] = {0};
				snprintf(uidstring, 4, "%d", uid);
				server_sned_to_client(info, uidstring);
			}
			handled = true;
		}

		if (!handled && longind) {
			info->client_msg = buffer;
			client_hdl(info);
		}

		memset(buffer, '\0', BUFFER_SIZE);
	}

	server_log("Client #%d has disconnected.\n", info->client_fd);

	if (longind) {
		server_sned_to_client(info, CMD_SERVER_CLOSED);
		server_online_remove(info);
		server_notify_disconn(info);
		free(info->client_user);
	}

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

		client->client_fd = accept(sock_fd, (struct sockaddr*) &(client->client_addr), (socklen_t*)&(client->client_addr_size));
		if (client->client_fd < 0) {
			wfree(client);
			continue;
		}

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

