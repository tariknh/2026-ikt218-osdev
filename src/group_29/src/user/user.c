#include "./user.h"
#include "user.h"

static UserData main_user_data;

void init_username() {
    string_copy("default", main_user_data.username, USER_MAX_USERNAME_LENGTH + 1);
}

int8_t user_set_username(const char username[]) {
    if (!username) return -1;
    return string_copy(username, main_user_data.username, USER_MAX_USERNAME_LENGTH + 1);
}

const char* user_get_username() {
    return main_user_data.username;
}