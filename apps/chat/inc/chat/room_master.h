#ifndef NGINZ_CHAT_ROOM_MASTER_H
#define NGINZ_CHAT_ROOM_MASTER_H

C_CAPSULE_START

#define ON_ASYNC_ROOM_CALL "on/async/room/call"
#define ON_ASYNC_ROOM_REPLY "on/async/room/reply"
int room_master_module_init();
int room_master_module_deinit();

C_CAPSULE_END

#endif // NGINZ_CHAT_ROOM_MASTER_H
