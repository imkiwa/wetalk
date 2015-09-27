#pragma once

#include "wetalk.h"
#include "user.h"
#include "server_cmd.h"

typedef struct client_info {
	int client_fd;
	int client_id;
	size_t client_addr_size;
	struct sockaddr_in client_addr;
	pthread_t client_thread;
	
	char *client_msg;
	user_info *client_user;
} client_info;


typedef void(*client_handler)(client_info *info);
typedef void(*online_foreach_callback)(client_info* info, void *data);

bool server_isrunning();
bool server_init(client_handler handler);
void server_loop();
void server_stop();
void server_online_foreach(online_foreach_callback fn, void *data);
void server_sned_to_client(client_info *info, const char *msg);


