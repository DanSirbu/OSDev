#include "serial.h"
#include "spinlock.h"

static inline int xchg(volatile int *lock_addr, int value)
{
	__asm__ volatile("xchg %0, %1"
			 : "=r"(value), "=m"(*lock_addr)
			 : "0"(value)
			 : "memory");
	return value;
}

#define FLAG_IF 0x200

//TODO, careful that the same cpu doesn't acquire it twice since it will never get it
void spinlock_acquire(int volatile *lock)
{
	/*if (get_flags() & FLAG_IF) {
		cli();
		debug_print("Disabled flags\n");
	}*/
	//Wait for lock, since xchg returns the old value of lock, it will return 0 when succeed
	while (xchg(lock, 1))
		;

	// From xv6
	// Tell the C compiler and the processor to not move loads or stores
	// past this point, to ensure that the critical section's memory
	// references happen after the lock is acquired.
	__sync_synchronize();
}
void spinlock_release(int volatile *lock)
{
	if (!*lock) {
		fail("Lock already released");
	}
	while (xchg(lock, 0) == 0)
		;
}