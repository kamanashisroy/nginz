#ifndef DEMOCHAT_FIBER_H
#define DEMOCHAT_FIBER_H

C_CAPSULE_START

/*
 * It sends ping to next process
 * @param pkt is the ping message
 */
NGINEZ_INLINE int pp_ping(aroop_txt_t*pkt);
/*
 * It responds to the ping message to the previous process
 * @param pkt is the pong message
 */
NGINEZ_INLINE int pp_pong(aroop_txt_t*pkt);

int pp_module_init();
int pp_module_deinit();

C_CAPSULE_END

#endif // DEMOCHAT_FIBER_H
