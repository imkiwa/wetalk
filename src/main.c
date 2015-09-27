#include "wetalk.h"

int main(int argc, char **argv) {
	if (argc == 1) {
		return wetalk_usage(argv[0]);
	}

	if (!strcmp(argv[1], "--daemon") || !strcmp(argv[1], "-d")) {
		return server_main();
	} else if (!strcmp(argv[1], "--client") || !strcmp(argv[1], "-c")) {
		return client_main(argc - 1, argv + 1);
	}

	return wetalk_usage(argv[0]);
}

