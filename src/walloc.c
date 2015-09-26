#include "wetalk.h"
#include "walloc.h"

void* walloc(size_t size) {
	void *p = malloc(size);
	if (!p) {
		wetalk_error("walloc: %s\n", strerror(errno));
	}

	memset(p, '\0', size);
	return p;
}

void wfree(void *ptr) {
	if (ptr) {
		free(ptr);
	}
}

