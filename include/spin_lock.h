#ifndef SPINLOCK_H
#define SPINLOCK_H

#include "types.h"

struct spinlock {
    unsigned int v;
};

static inline void spin_lock(struct spinlock *lock)
{
	while (__sync_lock_test_and_set(&lock->v, 1) != 0);
}

static inline void spin_unlock(struct spinlock *lock)
{
	__sync_lock_release(&lock->v);
}

#endif // SPINLOCK_H