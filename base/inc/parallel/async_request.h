#ifndef NGINZ_ASYNC_REQUEST_H
#define NGINZ_ASYNC_REQUEST_H

C_CAPSULE_START

int async_pm_call_master(int cb_token, aroop_txt_t*worker_hook, aroop_txt_t*master_hook, aroop_txt_t*args[]/* NULL terminated array */);
int async_pm_reply_worker(int destpid, int cb_token, aroop_txt_t*worker_hook, int success, aroop_txt_t*args[]/* NULL terminated array */);

C_CAPSULE_END

#endif // NGINZ_ASYNC_REQUEST_H
