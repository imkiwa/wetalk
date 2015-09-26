#include "wetalk.h"
#include "user.h"

#ifndef DB_CREATE_SQL
	#define DB_CREATE_SQL \
		"create table if not exists wetalk_user" \
		"(uid integer primary key autoincrement, username text not null, password text not null);"
#endif

#ifndef DB_CREATE_USER
	#define DB_CREATE_USER \
		"insert into wetalk_user(username, password) values('%s', '%s');"
#endif

#ifndef DB_FIND_BY_NAME_AND_PWD
	#define DB_FIND_BY_NAME_AND_PWD \
		"select * from wetalk_user where username = '%s' and password = '%s';"
#endif

#ifndef DB_FIND_USER_BY_ID
	#define DB_FIND_USER_BY_ID \
		"select * from wetalk_user where uid = %d;"
#endif

static sqlite3 *db;

static user_info* userdup(int uid, const char *name, const char *pwd) {
	user_info *info = (user_info*) walloc(sizeof(user_info));

	if (!name || !pwd) {
		wetalk_error("username or password could not be null!\n");
	}

	if (strlen(name) >= USERNAME_MAX || strlen(pwd) >= PASSWORD_MAX) {
		wetalk_error("username or password too long.\n");
	}

	info->uid = uid;
	info->username = strdup(name);
	info->password = strdup(pwd);

	return info;
}

static void user_uninit() {
	if (!db) {
		return;
	}

	sqlite3_close(db);
	db = NULL;
}

bool user_init() {
	if (db) {
		return true;
	}

	int r;
	r = sqlite3_open(DB_PATH, &db);
	if (r != SQLITE_OK) {
		return false;
	}
	atexit(user_uninit);

	sqlite3_exec(db, DB_CREATE_SQL, NULL, NULL, NULL);

	return true;
}

static int db_finduser_callback(void *data, int col_count, 
								char **col_values, char **col_name) 
{
	int uid;
	char *username;
	char *password;

	int i;
	for(i=0;i<col_count;i++) {
		if (!strcmp(col_name[i], "uid")) {
			uid = atoi(col_values[i]);
		} else if (!strcmp(col_name[i], "username")) {
			username = col_values[i];
		} else if (!strcmp(col_name[i], "password")) {
			password = col_values[i];
		}
	}

	user_info **info = (user_info**) data;
	*info = userdup(uid, username, password);
}

user_info* user_find(int uid) {
	char sql[BUFFER_SIZE] = {0};
	snprintf(sql, BUFFER_SIZE, DB_FIND_USER_BY_ID, uid);

	user_info* info = NULL;
	sqlite3_exec(db, sql, db_finduser_callback, (void*)&info, NULL);

	return info;
}

user_info* user_create(const char *name, const char *password) {
	char sql[BUFFER_SIZE] = {0};
	snprintf(sql, BUFFER_SIZE, DB_CREATE_USER, name, password);

	sqlite3_exec(db, sql, NULL, NULL, NULL);

	memset(sql, '\0', BUFFER_SIZE);
	snprintf(sql, BUFFER_SIZE, DB_FIND_BY_NAME_AND_PWD, name, password);

	user_info* info = NULL;
	sqlite3_exec(db, sql, db_finduser_callback, (void*)&info, NULL);

	return info;
}

user_info* user_login(int uid, const char *password) {
	user_info *info = user_find(uid);

	if (!info) {
		return NULL;
	}

	if (strcmp(password, info->password)) {
		user_release(info);
		return NULL;
	}

	return info;
}

void user_release(user_info *p) {
	wfree(p->username);
	wfree(p->password);
	wfree(p);
}

