OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
SECTIONS
 {
   . = 0xC0100000;
   .text ALIGN(0x1000) : AT(ADDR(.text)-0xC0000000)
   { 
     *(.boot_header)
     *(.boot_text)
     *(.text)
  }
   .data ALIGN(0x1000) : AT(ADDR(.data)-0xC0000000)
   { *(.data) }
   .bss ALIGN(0x1000) : AT(ADDR(.bss)-0xC0000000)
   { *(.bss); }
   . = ALIGN(0x400000);
   _kernel_end = .;
 }
