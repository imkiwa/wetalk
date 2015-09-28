#include "wetalk.h"
#include "user.h"
#include "server.h"

#include <locale.h>
#include <ncursesw/ncurses.h>
#include <readline/readline.h>

#ifndef INPUT_MAXLINE
#	define INPUT_MAXLINE 4
#endif

#undef MSG_APPEND_TO_HEAD

static bool client_main_run = true;
static bool is_printing = false;
static int window_y, window_x;
static char input[BUFFER_SIZE];

void draw_editarea() {
#ifndef MSG_APPEND_TO_HEAD
	move(window_y - INPUT_MAXLINE - 1, 0);
	hline('-', window_x);
#endif
	mvprintw(window_y - INPUT_MAXLINE, 0, "> ");
	clrtobot();
	refresh();
}

void msglist_add(char *fmt, ...) {
	char buffer[BUFFER_SIZE];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, BUFFER_SIZE, "%s", args);
	va_end(args);

	int x, y;

	getyx(stdscr, y, x);
	move(0, 0);

#ifdef MSG_APPEND_TO_HEAD /* 新消息添加到最上面 */
	insertln();
	printw("[NewMessage] %s", buffer);
	move(y, x);
	deleteln();
	clrtobot();
#else /* 新消息添加到最下面 */
	deleteln();
	move(window_y - INPUT_MAXLINE - 2, 0); // -2 输入行和分割线
	insertln();
	printw("[NewMessage] %s", buffer);
	move(y, x);
#endif

	refresh();
}

void* read_thread(void *args) {
	int bytes = 0;
	unsigned char buffer[BUFFER_SIZE];
	int sock = (int) args;

	while (client_main_run && (bytes = read(sock, buffer, BUFFER_SIZE - 1)) > 0) {
		buffer[bytes] = '\0';
		if (!strcmp(buffer, CMD_SERVER_CLOSED)) {
			break;
		}

		msglist_add(buffer);

		memset(buffer, '\0', BUFFER_SIZE);
	}

	client_main_run = false;
}

int client_main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "You must give me a ip address.\n");
		return wetalk_usage("wetalk");
	}

	int sock;
	struct sockaddr_in wetalk_server;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		wetalk_error("Failed to create socket\n");
	}

	memset(&wetalk_server, 0, sizeof(wetalk_server));      
	wetalk_server.sin_family = AF_INET;                
	wetalk_server.sin_addr.s_addr = inet_addr(argv[1]);
	wetalk_server.sin_port = htons(LISTEN_PORT);     

	if (connect(sock, (struct sockaddr *) &wetalk_server, sizeof(wetalk_server)) < 0) {
		wetalk_error("Failed to connect with server\n");
	}

	int r = strlen(CMD_LOGIN);
	if (write(sock, CMD_LOGIN, r) != r) {
		wetalk_error("Failed to send login command\n");
	}

	/*
	 * check login here
	 */

	setlocale(LC_ALL, "");
	initscr();
	raw();
	getmaxyx(stdscr, window_y, window_x);

	pthread_t readthr;
	if (pthread_create(&readthr, NULL, read_thread, (void*) sock) < 0) {
		wetalk_error("Failed to create thread\n");
	}

	// char *input = NULL;
	char input[BUFFER_SIZE] = {0};
	while(client_main_run) {
		draw_editarea();

		getnstr(input, BUFFER_SIZE - 1);
		r = strlen(input);
		input[r] = '\0';

		add_history(input);
		if (write(sock, input, r) != r) {
			wetalk_warning("Unexpected message has sent\n");
		}
	}

	client_main_run = false;

	r = strlen(CMD_EXIT);
	if (write(sock, CMD_EXIT, r) != r) {
		wetalk_warning("Unexpected exit command has sent\n");
	}
	close(sock);

	endwin();
	return 0;
}

#if 0
void* read_thread(void *args) {
	int bytes = 0;
	unsigned char buffer[BUFFER_SIZE];
	int sock = (int) args;

	while (client_main_run && (bytes = read(sock, buffer, BUFFER_SIZE - 1)) > 0) {
		buffer[bytes] = '\0';
		if (!strcmp(buffer, CMD_SERVER_CLOSED)) {
			break;
		}

		fprintf(stderr, "\n[!NewMessage!] %s\n", buffer);
		memset(buffer, '\0', BUFFER_SIZE);
	}

	client_main_run = false;
}



int client_main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "You must give me a ip address.\n");
		return wetalk_usage("wetalk");
	}

	int sock;
	struct sockaddr_in wetalk_server;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		wetalk_error("Failed to create socket\n");
	}

	memset(&wetalk_server, 0, sizeof(wetalk_server));      
	wetalk_server.sin_family = AF_INET;                
	wetalk_server.sin_addr.s_addr = inet_addr(argv[1]);
	wetalk_server.sin_port = htons(LISTEN_PORT);     

	if (connect(sock, (struct sockaddr *) &wetalk_server, sizeof(wetalk_server)) < 0) {
		wetalk_error("Failed to connect with server\n");
	}

	pthread_t readthr;
	if (pthread_create(&readthr, NULL, read_thread, (void*) sock) < 0) {
		wetalk_error("Failed to create thread\n");
	}

	int r = strlen(CMD_LOGIN);
	if (write(sock, CMD_LOGIN, r) != r) {
		wetalk_error("Failed to send login command\n");
	}
	
	char *input = NULL;
	while(client_main_run && (input = readline("")) != NULL) {
		if (*input) {
			add_history(input);
			r = strlen(input);
			if (write(sock, input, r) != r) {
				wetalk_warning("Unexpected message has sent\n");
			}
		}
		if (input) {
			free(input);
			input = NULL;
		}
	}

	client_main_run = false;

	r = strlen(CMD_EXIT);
	if (write(sock, CMD_EXIT, r) != r) {
		wetalk_warning("Unexpected exit command has sent\n");
	}
	close(sock);
	return 0;
}
#endif

