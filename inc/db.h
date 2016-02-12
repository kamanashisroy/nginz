#ifndef NGINZ_DB_H
#define NGINZ_DB_H

C_CAPSULE_START

int db_set(const char*key, const char*value);
int db_get(const char*key,aroop_txt_t*output);
int db_get_int(const char*key);
int db_set_int(const char*key, int value);


int db_module_init();
int db_module_deinit();

C_CAPSULE_END

#endif // NGINZ_DB_H
