#include "../include/types.h"
#include "../include/boot.h"
#include "../include/serial.h"

#define CMOS_PORT 0x70
#define CMOS_PORT_INOUT 0x71
#define IRQ_PIT 0x20

uint8_t read_cmos(u8 cmos_reg) {
	write_port(CMOS_PORT, cmos_reg); //Must always reselect before reading because it seems reading clears the selection
	return read_port(CMOS_PORT_INOUT);
}

void print_time() {
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
	kpanic_fmt("%d\n", value); //Default promotion takes care of casting
}
