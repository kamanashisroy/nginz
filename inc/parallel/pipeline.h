#ifndef DEMOCHAT_FIBER_H
#define DEMOCHAT_FIBER_H

C_CAPSULE_START

/*
 * It sends ping to next process
 * @param pkt is the ping message
 */
NGINZ_INLINE int pp_ping(aroop_txt_t*pkt);
/*
 * It sends socket to the next process
 * @param socket is the file descriptor
 */
NGINZ_INLINE int pp_pingmsg(int socket, aroop_txt_t*cmd);

/*
 * It responds to the ping message to the parent process
 * @param pkt is the pong message
 */
NGINZ_INLINE int pp_pong(aroop_txt_t*pkt);
/*
 * It responds to the ping message to the parent process with returning socket.
 * @param socket is the file descriptor
 */
NGINZ_INLINE int pp_pongmsg(int socket, aroop_txt_t*cmd);

/*
 * It checks if the process is master process.
 * @returns 1 if master
 */
NGINZ_INLINE int is_master();

int pp_module_init();
int pp_module_deinit();

C_CAPSULE_END

#endif // DEMOCHAT_FIBER_H
