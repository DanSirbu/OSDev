#include "time.h"
#include "boot.h"
#include "serial.h"
#include "types.h"
#include "io.h"
#include "trap.h"
#include "task.h"

#define CMOS_PORT 0x70
#define CMOS_PORT_INOUT 0x71
#define IRQ_PIT 0x20

uint8_t read_cmos(u8 cmos_reg)
{
	outb(CMOS_PORT,
	     cmos_reg); // Must always reselect before reading because it seems
	// reading clears the selection
	return inb(CMOS_PORT_INOUT);
}

void print_time()
{
	/* From https://wiki.osdev.org/CMOS#Accessing_CMOS_Registers
  Register  Contents
  0x00      Seconds
  0x02      Minutes
  0x04      Hours
  0x06      Weekday
  0x07      Day of Month
  0x08      Month
  0x09      Year
  0x32      Century (maybe)
  0x0A      Status Register A
  0x0B      Status Register B
  */
	uint8_t value = read_cmos(0);
	debug_print("%d\n", value); // Default promotion takes care of casting
}
#define PIT_BASE_FREQUENCY 1193180
#define PIT_CMD 0x43
#define PIT0_DATA 0x40

#define BYTE0(a) ((a)&0xFF)
#define BYTE1(a) ((a) >> 8 & 0xFF)
#define BYTE2(a) ((a) >> 16 & 0xFF)
#define BYTE3(a) ((a) >> 24 & 0xFF)

void timer_interrupt();

void timer_init(uint32_t frequency)
{
	uint32_t div = PIT_BASE_FREQUENCY / frequency;
	outb(PIT_CMD, 0x36);
	outb(PIT0_DATA, BYTE0(div));
	outb(PIT0_DATA, BYTE1(div));
	register_isr_handler(TRAP_TIMER, timer_interrupt);
}
extern void sendEOI(uint32_t interrupt_no);
uint32_t ticks = 0; //Since frequency is 1000, this is a millisecond
uint32_t unix_time = 0; //Seconds
void timer_interrupt(int_regs_t *regs)
{
	ticks++;
	if (ticks % 1000 == 0) {
		unix_time++;
	}

	if (ticks % 100 == 0) {
		sendEOI(32);
		schedule();
	}
}

/*
Note that you should probably have:
A thread state for each thread (saying if it's "running", "ready to run", or blocked for some reason).
The "switch_to(thread)" function that does nothing if the thread being switched to is currently running; or (otherwise):
saves the currently running thread's register contents, etc
checks if the currently running thread's state is "running" and if it is, puts the currently running thread back into the "ready to run" state", and puts it on whatever data structure the scheduler uses to keep track of "ready to run" threads
sets the state of the thread being switched to to "running", and loads its register contents, etc.
A "find_thread_to_switch_to()" function that chooses a thread to switch to (using whatever data structure the scheduler uses to keep track of "ready to run" threads), removes the selected thread from the scheduler's data structure, and then calls the "switch_to(thread)" function.
A "block_thread(reason)" function which sets the currently running thread's state to whatever the reason is, then calls the "find_thread_to_switch_to()" function.
An "unblock_thread(thread)" function which sets a thread's state to "read to run", then decides if the thread being unblocked should preempt the currently running thread. If the thread being unblocked shouldn't preempt then it puts the thread on whatever data structure the scheduler uses to keep track of "ready to run" threads; and if the thread being unblocked should preempt it calls the "switch_to(thread)" function instead (to cause an immediate task switch, without bothering with the unnecessary overhead of the "find_thread_to_switch_to()" function).
*/

//Handlers cannot call schedule