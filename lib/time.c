#include "time.h"
#include "boot.h"
#include "serial.h"
#include "types.h"
#include "io.h"
#include "trap.h"

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
	kpanic_fmt("%d\n", value); // Default promotion takes care of casting
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
	register_handler(TRAP_TIMER, timer_interrupt);
}

uint32_t ticks = 0; //Since frequency is 1000, this is a millisecond
uint32_t unix_time = 0; //Seconds
void timer_interrupt()
{
	ticks++;
	if (ticks % 1000 == 0) {
		unix_time++;
	}

	if (ticks % 100 == 0) {
		//TODO schedule
		//1. Write to TSS
		//2. Switch to new stack
		//   - What if the new stack wasn't from an interrupt?
		//   	- Then it was a kernel thread not a user process
		//That's it?

		
	}
}