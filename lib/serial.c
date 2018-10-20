//Taken from https://wiki.osdev.org/Serial_Ports
#include <stdarg.h>
#include "../include/serial.h"

#define PORT 0x3f8   /* COM1 */

extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);

#define outb(port, data) write_port(port, data)
#define inb(port) read_port(port)

void init_serial() {
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received() {
   return inb(PORT + 5) & 1;
}
 
char read_serial() {
   while (serial_received() == 0);
 
   return inb(PORT);
}

int is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}
 
void write_serial(char a) {
   while (is_transmit_empty() == 0);
 
   outb(PORT,a);
}
void kpanic_fmt(char *message, ...) {
    va_list args;
    va_start(args, message);
    
    int i = 0;
    while(message[i] != '\0') {
        if(message[i] == '%') {
            i++;
            if(message[i] == '%') {
                write_serial('%');
            }
            else if(message[i] == 'x') {
                char buf[256];
                itoa(va_arg(args, u64), buf, 16);
                kpanic(buf);
            } else if(message[i] == 'd') {
                char buf[256];
                itoa(va_arg(args, u64), buf, 10);
                kpanic(buf);
            } else if(message[i] == 's') {
                kpanic(va_arg(args, char*));
            } else if(message[i] == 'p') {
                char buf[256];
                itoa((size_t) va_arg(args, size_t*), buf, 16);
                kpanic(HEX_PREFIX);
                kpanic(buf);
            }
        } else {
            write_serial(message[i]);
        }
        i++;
    }
}
void kpanic(char *message) {
    int i = 0;
    while(message[i] != '\0') {
        write_serial(message[i]);
        i++;
    }
}