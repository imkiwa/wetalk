#pragma once

#include "wetalk.h"

typedef struct user_info {
	int uid;
	char *password;
	char *username;
} user_info;


bool user_init();

user_info* user_find(int uid);
user_info* user_create(const char *name, const char *password);
user_info* user_login(int uid, const char *password);

void user_release(user_info *p);
