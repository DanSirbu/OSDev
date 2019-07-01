#include "trap.h"
#include "task.h"
#include "debug.h"

extern void sendEOI(uint32_t interrupt_no);

static isr_handler_t interrupt_handlers[256];

static char *exceptions_string[32] = {
	"Divide-by-zero",
	"Debug",
	"Non-maskable Interrupt",
	"Breakpoint",
	"Overflow",
	"Bound Range Exceeded",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Invalid TSS",
	"Segment Not Present",
	"Stack-Segment Fault",
	"General Protection Fault",
	"Page Fault",
	"Reserved",
	"x87 Floating-Point Exception",
	"Alignment Check",
	"Machine Check",
	"SIMD Floating-Point Exception",
	"Virtualization Exception", // 0x14
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Security Exception", // 30 (0x1E)
	"Reserved" // 31 (0x1F)
};

static char *interrupts_string[32] = { "Programmable Interrupt Timer",
				       "Keyboard",
				       "Cascade",
				       "Com2",
				       "Com1",
				       "LPT2",
				       "Floppy disk",
				       "LPT1",
				       "CMOS Real time clock",
				       "Peripherals",
				       "Peripherals",
				       "Peripherals",
				       "PS2 Mouse",
				       "FPU/Coprocessor",
				       "Primary ATA Hard Disk",
				       "Secondary ATA Hard Disk" };
uint32_t NUM_INTERRUPT_STRINGS =
	sizeof(interrupts_string) / sizeof(interrupts_string[0]);
void register_isr_handler(int interrupt_no, isr_handler_t handler)
{
	interrupt_handlers[interrupt_no] = handler;
}

void interrupt_handler(int_regs_t regs)
{
	set_int_regs(&regs);
	if (regs.interrupt_no < 32) {
		debug_print("Exception %d (%s) at 0x%x, error 0x%x\n",
			    regs.interrupt_no,
			    exceptions_string[regs.interrupt_no], regs.eip,
			    regs.error_code);
		dump_registers(&regs);
	} else if (interrupt_handlers[regs.interrupt_no] == NULL &&
		   (regs.interrupt_no - 32) < NUM_INTERRUPT_STRINGS) {
		debug_print("Interrupt %d (%s) at 0x%x, error 0x%x\n",
			    regs.interrupt_no - 32,
			    interrupts_string[regs.interrupt_no - 32], regs.eip,
			    regs.error_code);
	}

	//Send the interrupt if there is a handler
	if (interrupt_handlers[regs.interrupt_no] != NULL) {
		void (*func)(int_regs_t *) =
			interrupt_handlers[regs.interrupt_no];
		(*func)(&regs);
	} else {
		debug_print("No handler for interrupt %d\n", regs.interrupt_no);
	}

	if (regs.interrupt_no >= PIC_REMAPPED_START &&
	    regs.interrupt_no <= PIC_REMAPPED_END &&
	    regs.interrupt_no != TRAP_TIMER) {
		sendEOI(regs.interrupt_no - PIC_REMAPPED_START);
	}
}