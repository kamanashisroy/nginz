#ifndef DEMOCHAT_FIBER_H
#define DEMOCHAT_FIBER_H

C_CAPSULE_START

enum {
	NGINZ_PIPELINE_SIGNATURE = 0x3234,
};

/**
 * @brief It sends message to a destination process.
 * It sends broadcast message if the dnid is 0.
 * @param dnid is the destination process id
 * @param pkt is the message
 */
NGINZ_INLINE int pp_send(int dnid, aroop_txt_t*pkt);

NGINZ_INLINE int pp_next_nid();

NGINZ_INLINE int pp_next_worker_nid(int nid);

/**
 * It checks if the process is master process.
 * @returns 1 if master
 */
NGINZ_INLINE int is_master();

/**
 * It gets the raw socket to send data to the process.
 * @nid is the process id(pid).
 * @returns -1 on failure, otherwise if returns the raw socket file descriptor.
 */
NGINZ_INLINE int pp_get_raw_fd(int nid);

int pp_module_init();
int pp_module_deinit();

C_CAPSULE_END

#endif // DEMOCHAT_FIBER_H
