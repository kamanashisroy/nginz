#ifndef NGINZ_BROADCAST_H
#define NGINZ_BROADCAST_H

C_CAPSULE_START

int broadcast_room_join(struct chat_connection*chat, aroop_txt_t*room_name);
int broadcast_room_leave(struct chat_connection*chat);
int broadcast_add_room(aroop_txt_t*room_name);
int broadcast_module_init();
int broadcast_module_deinit();

C_CAPSULE_END

#endif // NGINZ_BROADCAST_H
