#include "../include/types.h"
#include "../include/serial.h"

extern char kernel_end;
void kmalloc() {
    kpanic_fmt("0x%x\n", (u64) (u32) (void *) &kernel_end);
}