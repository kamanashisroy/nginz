#ifndef DEMOCHAT_FIBER_H
#define DEMOCHAT_FIBER_H

C_CAPSULE_START

/**
 * @brief It sends message to a destination process.
 * It sends broadcast message if the dnid is 0.
 * @param dnid is the destination process id
 * @param pkt is the message
 */
NGINZ_INLINE int pp_send(int dnid, aroop_txt_t*pkt);

NGINZ_INLINE int pp_next_nid();

/**
 * It checks if the process is master process.
 * @returns 1 if master
 */
NGINZ_INLINE int is_master();

int pp_module_init();
int pp_module_deinit();

C_CAPSULE_END

#endif // DEMOCHAT_FIBER_H
