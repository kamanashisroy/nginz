#ifndef NGINZ_ASYNC_DB_MASTER_H
#define NGINZ_ASYNC_DB_MASTER_H

C_CAPSULE_START

int noasync_db_get(aroop_txt_t*key, aroop_txt_t*val);
int async_db_master_init(int cb_token, aroop_txt_t*cb_hook, aroop_txt_t*key, int intval);
int async_db_master_deinit(int cb_token, aroop_txt_t*cb_hook, aroop_txt_t*key);

C_CAPSULE_END

#endif // NGINZ_ASYNC_DB_MASTER_H
