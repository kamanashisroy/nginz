#ifndef DEMOCHAT_FIBER_H
#define DEMOCHAT_FIBER_H

C_CAPSULE_START

enum fiber_status {
	FIBER_STATUS_CREATED = 0,
}

int register_fiber(int (fiber*)(int status));
int unregister_fiber(int (fiber*)(int status));

C_CAPSULE_END

#endif // DEMOCHAT_FIBER_H
