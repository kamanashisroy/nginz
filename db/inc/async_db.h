#ifndef NGINZ_ASYNC_DB_H
#define NGINZ_ASYNC_DB_H

C_CAPSULE_START

int async_db_set_if_null(int cb_token, aroop_txt_t*cb_hook, aroop_txt_t*key, int intval);
int async_db_get(int cb_token, aroop_txt_t*cb_hook, aroop_txt_t*key);
int async_db_increment(int cb_token, aroop_txt_t*cb_hook, aroop_txt_t*key, int intval, int increment);
int async_db_unset(int cb_token, aroop_txt_t*cb_hook, aroop_txt_t*key);
int async_db_compare_and_swap(int cb_token, aroop_txt_t*cb_hook, aroop_txt_t*key, aroop_txt_t*newval, aroop_txt_t*oldval);

C_CAPSULE_END

#endif // NGINZ_ASYNC_DB_H
