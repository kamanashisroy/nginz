#ifndef NGINZ_LAZY_CALL_H
#define NGINZ_LAZY_CALL_H

C_CAPSULE_START

/**
 * It calls the go_lazy after the execution of current fiber. In fact it is executed in separate fiber.
 */
int lazy_call(int go_lazy(void*data), void*data);

/**
 * It unrefs the object lazily. @see lazy_call() behavior.
 */
int lazy_cleanup(void*obj_data);

C_CAPSULE_END

#endif // NGINZ_LAZY_CALL_H
