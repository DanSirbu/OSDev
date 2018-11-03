#include "../include/trap.h"

extern void sendEOI(uint32_t interrupt_no);

static void *interrupt_handlers[256];

void register_handler(int interrupt_no, void *handler)
{
	interrupt_handlers[interrupt_no] = handler;
}

void interrupt_handler(u32 cr2, u32 edi, u32 esi, u32 ebp, u32 esp, u32 ebx,
		       u32 edx, u32 ecx, u32 eax, const u32 interrupt_no,
		       u32 error_code, size_t eip)
{
	if (interrupt_no < 32) {
		kpanic_fmt("Exception %d (%s) at 0x%x, error %d\n",
			   interrupt_no, exceptions_string[interrupt_no], eip,
			   error_code);
	} else {
		kpanic_fmt("Interrupt %d (%s) at 0x%x, error %d\n",
			   interrupt_no - 32,
			   interrupts_string[interrupt_no - 32], eip,
			   error_code);
	}

	//Send the interrupt if there is a handler
	if (interrupt_handlers[interrupt_no] != NULL) {
		void (*func)(uint32_t) = interrupt_handlers[interrupt_no];
		(*func)(error_code);
	} else {
		fail("No handler for interrupt\n");
	}

	if (interrupt_no >= 32) {
		sendEOI(interrupt_no);
	}
}