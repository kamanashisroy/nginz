#ifndef NGINZ_CHAT_ROOM_H
#define NGINZ_CHAT_ROOM_H

C_CAPSULE_START

int chat_room_set_user_count(aroop_txt_t*my_room, int user_count);
int chat_room_get_pid(aroop_txt_t*my_room, int token, aroop_txt_t*callback_hook);

int room_module_init();
int room_module_deinit();

C_CAPSULE_END

#endif // NGINZ_CHAT_ROOM_H
