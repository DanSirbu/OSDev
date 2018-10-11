#include "../include/e1000.h"
#include "../include/types.h"
#include "../include/kmalloc.h"
#include "../include/serial.h"

//TODO, get these values using PCI
#define ETHERNET_IO_BASE 0xc03f
#define ETHERNET_MEM_BASE 0xFEBC0000
//                        0xC0100000

uint32_t readCommand(uint16_t offset) {
    return *((uint32_t *) ETHERNET_MEM_BASE + offset);
}
void writeCommand(uint16_t addr, uint32_t value) {
    uint32_t *ptr = (uint32_t*) ETHERNET_MEM_BASE+addr;
    *ptr = value;
}
bool detectEEProm()
{
    uint32_t val = 0;
    writeCommand(REG_EEPROM, 0x1); 
    bool eerprom_exists = false;

    for(int i = 0; i < 1000 && ! eerprom_exists; i++)
    {
            val = readCommand( REG_EEPROM);
            if(val & 0x10)
                    eerprom_exists = true;
            else
                    eerprom_exists = false;
    }
    return eerprom_exists;
}
uint16_t eeprom_read(uint8_t addr) {
    //bit 0 = tells hardware to start the read
    //15-8 = read address
    writeCommand(REG_EEPROM, (((uint32_t) addr) << 8) | 1);
    
    //Bit 4 gets set to one when data received
    uint32_t eepromValue = readCommand(REG_EEPROM);
    while(!(eepromValue & (1 << 4))); //Wait for data

    //Bit 31-16 contain the data
    return (uint16_t) ((eepromValue >> 16) & 0xFFFF);
}
//To decide on the best routing, routers use: IGMP, BGP
//IP gives packet to right computer
//TCP/UDP specify the program (port)+checksum

//Google works on concept of backlinks (how many people put a link to website, so it means it gets used and its not spam, (probably))

//Application layer - 
//Presentation layer - Web servers, hyperlinks, pages, HTTP

//Session layer - Open Connection, PassInfo, close connection
//Transport layer - TCP/UDP
//Netowork layer - Switching/routing 
//Data link layer - MAC
//Physical layer - physical bits
uint16_t pciCheckVendor(uint8_t bus, uint8_t slot);

//Temporary for now
void ethernet_main() {
    //mem_base = pciConfigHeader->getPCIBar( PCI_BAR_MEM) & ~3; 
    /*for(int i = 0; i < 10; i++) {
        uint16_t vendor1 = pciCheckVendor(0, i);
    }*/
    kpanic_fmt("detectEEProm %x\n", (uint64_t) detectEEProm());
    //kpanic_fmt("mac0 %x\n", (uint64_t) eeprom_read(0));
}
 
uint32_t inportl( uint16_t p_port)
{
    uint32_t l_ret;
    asm volatile ("inl %1, %0" : "=a" (l_ret) : "dN" (p_port));
    return l_ret;
}
void outportl (uint16_t p_port,uint32_t p_data)
{
    asm volatile ("outl %1, %0" : : "dN" (p_port), "a" (p_data));
}

#define PCI_ADDR 0xCF8
#define PCI_DATA 0xCFC
//Slot = device number
uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t function, uint8_t reg) {
    uint32_t address_to_read = 0;
    uint32_t busl = (uint32_t) bus;
    uint32_t slotl = (uint32_t) slot;
    uint32_t functionl = (uint32_t) function;
    
    address_to_read |= 1 << 31; //Enable bit
    address_to_read |= busl << 16;
    address_to_read |= (slotl & 0x1F) << 11;
    address_to_read |= (functionl & 0x3) << 8;
    address_to_read |= (reg & 0xFC);

    outportl(PCI_ADDR, address_to_read);

    //Second bit of reg selects second word
    //reg = 0xFF
    return (inportl(PCI_DATA) >> (reg & 0x2) * 8) & 0xFFFF;
}
#define NON_EXISTANT_DEVICE 0xFFFF
uint16_t pciCheckVendor(uint8_t bus, uint8_t slot) {
    uint16_t vendor, device;
    vendor = pciConfigReadWord(bus, slot, 0, 0);
    if(vendor != NON_EXISTANT_DEVICE) {
        device = pciConfigReadWord(bus, slot, 0, 2); //High word of offset 0
    }
    return vendor;
}