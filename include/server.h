#pragma once

#include "wetalk.h"
#include "user.h"

#ifndef LISTEN_PORT
#	define LISTEN_PORT 16650
#endif

#ifndef CMD_SERVER_CLOSED
#	define CMD_SERVER_CLOSED "sever-closed"
#endif

#ifndef CMD_SUCCESS
#	define CMD_SUCCESS "success"
#endif

#ifndef CMD_EXIT
#	define CMD_EXIT "exit"
#endif

typedef struct client_info {
	int client_fd;
	int client_addr_size;
	struct sockaddr_in client_addr;
	pthread_t client_thread;
	
	char *client_msg;
	user_info *client_user;
} client_info;

typedef void(*client_handler)(client_info *info);

bool server_isrunning();
bool server_init(client_handler handler);
void server_loop();
void server_stop();


