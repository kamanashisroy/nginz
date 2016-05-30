#ifndef NGINZ_FIBER_INTERNAL_H
#define NGINZ_FIBER_INTERNAL_H

C_CAPSULE_START


/**
 * It stops all the fibers, eventually quiting the application
 */
int fiber_quit();
// TODO fiber suspend function

int fiber_module_init();
int fiber_module_deinit();

C_CAPSULE_END

#endif // NGINZ_FIBER_INTERNAL_H
