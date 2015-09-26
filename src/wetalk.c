#include "wetalk.h"

static void pid_uninit() {
	remove(PID_FILE);
}

static int wetalk_print(const char *prompt, FILE *fp, const char *fmt, va_list arg) {
	fprintf(fp, "%s", prompt);
	vfprintf(fp, fmt, arg);
}

bool pid_init() {
	if (access(PID_FILE, R_OK) == 0) {
		return false;
	}

	FILE *fp = fopen(PID_FILE, "w");
	fprintf(fp, "%d", getpid());
	fclose(fp);

	atexit(pid_uninit);
	return true;
}

void wetalk_warning(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	wetalk_print("wetalk: warning: ", stderr, fmt, args);
	va_end(args);

	fprintf(stderr, "\n");
}

void wetalk_error(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	wetalk_print("wetalk: error: ", stderr, fmt, args);
	va_end(args);

	fprintf(stderr, "\n");

	exit(1);
}

