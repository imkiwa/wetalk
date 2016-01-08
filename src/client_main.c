#include "client.h"

#include <locale.h>
#include <ncurses.h>

#ifndef INPUT_MAXLINE
#	define INPUT_MAXLINE 4
#endif

static bool client_main_run;
static int window_y, window_x;

static void draw_editarea() {
	move(window_y - INPUT_MAXLINE - 1, 0);
	hline('-', window_x);
	mvprintw(window_y - INPUT_MAXLINE, 0, "> ");
	clrtobot();
	refresh();
}

static int compute_line(const char *msg) {
	int r;
	int l = strlen(msg);

	r = l / window_x;
	if (l % window_x != 0) {
		++r;
	}
	return r;
}

static void msglist_add(const char *msg) {
	int x, y;
	getyx(stdscr, y, x);
	move(0, 0);

	int n = compute_line(msg);
	int i;

	for (i = 0; i < n; ++i) {
		deleteln();
	}

	int newy = window_y - INPUT_MAXLINE - 2 - n + 1;
	move(newy, 0);
	for (i = 0; i < n; ++i) {
		insertln();
	}
	printw("%s", msg);
	move(y, x);

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
	if (argc < 4) {
		fprintf(stderr, "Usage: wetalk -c <ip> <uid> <password>\n"
						"Or   : wetalk -r <ip> <username> <password>\n");
		return wetalk_usage("wetalk");
	}

	int uid = 0;
	char *ipaddr = argv[1];
	char *password = argv[3];

	if (strlen(password) >= PASSWORD_MAX) {
		fprintf(stderr, "错误的密码长度\n");
		return 1;
	}

	if (!client_init(argv[1], on_msg_recv)) {
		wetalk_error("初始化客户端失败\n");
	}

	if (!strcmp(argv[0], "-r")) {
		uid = client_register(argv[2], password);
		if (uid > 0) {
			fprintf(stderr, "注册成功，您的 uid 为: %d\n这是以后登陆时必要的信息，请牢记\n回车键以继续", uid);
			getchar();
		} else {
			fprintf(stderr, "注册失败\n");
			return 1;
		}
	} else {
		uid = atoi(argv[2]);
		if (uid <= 0) {
			fprintf(stderr, "错误的uid\n");
			return 1;
		}
	}

	if (!client_login(uid, password)) {
		wetalk_error("登陆失败\n");
	}

	client_main_run = true;
	int r;

	setlocale(LC_ALL, "");
	initscr();
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

