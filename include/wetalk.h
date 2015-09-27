#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <pthread.h>
#include <sqlite3.h>

#ifndef PID_FILE
#	define PID_FILE "/tmp/wetalk.pid"
#endif

#ifndef DB_PATH
#	define DB_PATH "/tmp/wetalk.db"
#endif

#ifndef BUFFER_SIZE
#	define BUFFER_SIZE 1024
#endif

#ifndef USERNAME_MAX
#	define USERNAME_MAX 64
#endif

#ifndef PASSWORD_MAX
#	define PASSWORD_MAX 16
#endif

#ifndef CLIENT_MAX
#	define CLIENT_MAX 1024
#endif

bool pid_init();

void wetalk_warning(const char *fmt, ...);
void wetalk_error(const char *fmt, ...);
int wetalk_usage(char *prog);

int server_main();
int client_main(int argc, char **argv);
