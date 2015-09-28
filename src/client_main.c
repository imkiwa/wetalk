#include "client.h"

#include <locale.h>
#include <ncursesw/ncurses.h>

#ifndef INPUT_MAXLINE
#	define INPUT_MAXLINE 4
#endif

#undef MSG_APPEND_TO_HEAD

static bool client_main_run;
static int window_y, window_x;

static void draw_editarea() {
	#ifndef MSG_APPEND_TO_HEAD
		move(window_y - INPUT_MAXLINE - 1, 0);
		hline('-', window_x);
	#endif

	mvprintw(window_y - INPUT_MAXLINE, 0, "> ");
	clrtobot();
	refresh();
}

static void msglist_add(const char *msg) {
	int x, y;
	getyx(stdscr, y, x);
	move(0, 0);

	#ifdef MSG_APPEND_TO_HEAD /* 新消息添加到最上面 */
		insertln();
		printw("[NewMessage] %s", msg);
		move(y, x);
		deleteln();
		clrtobot();
	#else /* 新消息添加到最下面 */
		deleteln();
		move(window_y - INPUT_MAXLINE - 2, 0); // -2 输入行和分割线
		insertln();
		printw("[NewMessage] %s", msg);
		move(y, x);
	#endif

	refresh();
}

static void on_msg_recv(const char *msg) {
	if (msg == NULL)  { /* server closed */
		client_main_run = false;
		return;
	}

	msglist_add(msg);
}

int client_main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "You must give me a ip address.\n");
		return wetalk_usage("wetalk");
	}

	if (!client_init(argv[1], on_msg_recv)) {
		wetalk_error("Failed to init client\n");
	}

	if (!client_login(0, "")) {
		wetalk_error("Failed to login\n");
	}

	client_main_run = true;
	int r;

	setlocale(LC_ALL, "");
	initscr();
	raw();
	getmaxyx(stdscr, window_y, window_x);

	char input[BUFFER_SIZE] = {0};
	while(client_main_run) {
		draw_editarea();

		getnstr(input, BUFFER_SIZE - 1);
		r = strlen(input);
		input[r] = '\0';

		if (r > 0) {
			client_send(input);
		}
	}

	endwin();
	return 0;
}

