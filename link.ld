OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
SECTIONS
 {
   . = 0xC0100000;
   .text ALIGN(0x1000) : AT(ADDR(.text)-0xC0000000)
   { *(.text) }
   .data ALIGN(0x1000) : AT(ADDR(.data)-0xC0000000)
   { *(.data) }
   .bss ALIGN(0x1000) : AT(ADDR(.bss)-0xC0000000)
   { *(.bss)  }
 }