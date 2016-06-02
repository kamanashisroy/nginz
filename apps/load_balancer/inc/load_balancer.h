#ifndef NGINZ_LOAD_BALANCER_H
#define NGINZ_LOAD_BALANCER_H

C_CAPSULE_START

struct load_balancer {
	int nid;
};

int load_balancer_setup(struct load_balancer*lb);
int load_balancer_destroy(struct load_balancer*lb);
int load_balancer_next(struct load_balancer*lb);

C_CAPSULE_END

#endif // NGINZ_LOAD_BALANCER_H
