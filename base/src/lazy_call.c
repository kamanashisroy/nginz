

#include "aroop/aroop_core.h"
#include "aroop/opp/opp_factory.h"
#include "nginz_config.h"
#include "fiber.h"
#include "lazy_call.h"
#include "lazy_call_internal.h"

C_CAPSULE_START

static int lazy_stack_count = 0;
static int (*lazy_stack[NGINZ_LAZY_STACK_SIZE])(void*data);
static void*lazy_stack_data[NGINZ_LAZY_STACK_SIZE];
int lazy_call(int go_lazy(void*data), void*data) {
	aroop_assert(go_lazy);
	aroop_assert(lazy_stack_count < NGINZ_LAZY_STACK_SIZE);
	lazy_stack[lazy_stack_count] = go_lazy;
	lazy_stack_data[lazy_stack_count] = data;
	lazy_stack_count++;
	return 0;
}

static int lazy_cleanup_count = 0;
static void*lazy_cleanup_objects[NGINZ_LAZY_OBJECT_QUEUE_SIZE];
int lazy_cleanup(void*obj_data) {
	aroop_assert(obj_data);
	aroop_assert(lazy_cleanup_count < NGINZ_LAZY_OBJECT_QUEUE_SIZE);
	if(lazy_cleanup_count == NGINZ_LAZY_OBJECT_QUEUE_SIZE)
		return -1;
	lazy_cleanup_objects[lazy_cleanup_count++] = obj_data;
	return 0;
}

static int lazy_call_step(int status) {
	int i = lazy_stack_count;
	while(i--) {
		lazy_stack[i](lazy_stack_data[i]);
	}
	lazy_stack_count = 0;
	i = lazy_cleanup_count;
	while(i--) {
		OPPUNREF(lazy_cleanup_objects[i]);
	}
	lazy_cleanup_count = 0;
	return 0;
}

int lazy_call_module_init() {
	register_fiber(lazy_call_step);
	return 0;
}

int lazy_call_module_deinit() {
	register_fiber(lazy_call_step);
	return 0;
}

C_CAPSULE_END


