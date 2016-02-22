#ifndef NGINZ_CONFIG_H
#define NGINZ_CONFIG_H

C_CAPSULE_START

#define NGINZ_INLINE inline

/**
 * Parallel computing
 */
#define NGINZ_NUMBER_OF_PROCESSORS 5

/**
 * Event loop configuration
 */
#define MAX_POLL_FD 10000
#define NGINZ_POLL_LISTEN_FLAGS POLLIN | POLLPRI | POLLHUP
#define NGINZ_POLL_ALL_FLAGS POLLIN

/**
 * Protocol implemenation
 */
#define NGINZ_MAX_PROTO 4
#define NGINZ_CHAT_PORT 9399
#define NGINZ_HTTP_PORT 80

/**
 * TCP listen config
 */
#define NGINZ_TCP_LISTENER_BACKLOG 1024 /* XXX we are setting this too high */

/**
 * object pool factory
 */
#define NGINZ_FACTORY_CREATE(obuff, psize, objsize, callback) ({OPP_PFACTORY_CREATE_FULL(obuff, psize, objsize, 1, OPPF_SWEEP_ON_UNREF, callback);})
#define NGINZ_HASHABLE_FACTORY_CREATE(obuff, psize, objsize, callback) ({OPP_PFACTORY_CREATE_FULL(obuff, psize, objsize, 1, OPPF_EXTENDED | OPPF_SWEEP_ON_UNREF, callback);})
#define NGINZ_SEARCHABLE_FACTORY_CREATE(obuff, psize, objsize, callback) ({OPP_PFACTORY_CREATE_FULL(obuff, psize, objsize, 1, OPPF_SEARCHABLE | OPPF_EXTENDED | OPPF_SWEEP_ON_UNREF, callback);})

/**
 * Chat server configuration
 */
#define NGINZ_MAX_CHAT_USER_NAME_SIZE 32
#define NGINZ_MAX_CHAT_ROOM_NAME_SIZE 32
#define NGINZ_MAX_CHAT_MSG_SIZE 255
#define NGINZ_MAX_WEBCHAT_MSG_SIZE (NGINZ_MAX_CHAT_MSG_SIZE+64)
#define NGINZ_MAX_WEBCHAT_SID_SIZE 32
#define NGINZ_MAX_COMMANDS 16

/**
 * Http server configuration
 */
#define NGINZ_MAX_HTTP_MSG_SIZE 1024
#define NGINZ_MAX_HTTP_HEADER_SIZE 1024

//#define NGINZ_EVENT_DEBUG 

#ifdef AROOP_MODULE_NAME
#undef AROOP_MODULE_NAME
#endif
#define AROOP_MODULE_NAME "NginZ"

C_CAPSULE_END

#endif // NGINZ_CONFIG_H
