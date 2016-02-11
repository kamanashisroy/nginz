#ifndef NGINZ_CHAT_USER_H
#define NGINZ_CHAT_USER_H


C_CAPSULE_START

int try_login(aroop_txt_t*name);
int logoff_user(aroop_txt_t*name);

C_CAPSULE_END

#endif // NGINZ_CHAT_USER_H
