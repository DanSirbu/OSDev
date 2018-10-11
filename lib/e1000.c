#include "../include/e1000.h"
#include "../include/types.h"
#include "../include/kmalloc.h"
#include "../include/serial.h"

//TODO, get these values using PCI
#define ETHERNET_IO_BASE 0xc03f
#define ETHERNET_MEM_BASE 0xFEBC0000
//                        0xC0100000
uint8_t mac[6];
uint32_t readCommand(uint16_t offset) {
    return *((uint32_t *) (ETHERNET_MEM_BASE + offset));
}
void writeCommand(uint16_t addr, uint32_t value) {
    uint32_t *ptr = (uint32_t*) (ETHERNET_MEM_BASE+addr);
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
void readMacAddress() {
    uint16_t temp = eeprom_read(0);
    mac[0] = temp & 0xFF;
    mac[1] = temp >> 8;
    temp = eeprom_read(1);
    mac[2] = temp & 0xFF;
    mac[3] = temp >> 8;
    temp = eeprom_read(2);
    mac[4] = temp & 0xFF;
    mac[5] = temp >> 8;
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
    readMacAddress();

    kpanic_fmt("Mac: 0x%x", (uint32_t) mac[0]);
    kpanic_fmt(" 0x%x", (uint32_t) mac[1]);
    kpanic_fmt(" 0x%x", (uint32_t) mac[2]);
    kpanic_fmt(" 0x%x", (uint32_t) mac[3]);
    kpanic_fmt(" 0x%x", (uint32_t) mac[4]);
    kpanic_fmt(" 0x%x\n", (uint32_t) mac[5]);

    //kpanic_fmt("mac0 %x\n", (uint64_t) eeprom_read(0));
}
void rxinit() {
    //NIC works with physical addresses
    void *ptr;
    struct e1000_rx_desc *descs;
    struct e1000_rx_desc *rx_desc[E1000_NUM_RX_DESC];
    //Allocate buffers for receive descriptors
    ptr = 0;//malloc(sizeof(e1000_rx_desc) * E1000_NUM_RX_DESC + 16; //Why + 16?

    descs = (struct e1000_rx_desc*) ptr;
    for(int i = 0; i < E1000_NUM_RX_DESC; i++) {
        rx_desc[i] = (struct e1000_rx_desc*) descs + i; //TODO check pointer arithmetic for structs
        rx_desc[i]->addr = kmalloc(8192 + 16); // malloc 2**13 + 16 TODO
        rx_desc[i]->status = 0;
    }
    //Here we set the buffer size, which will be 8192
    //writeCommand(REG_RCTRL, ); //Table 13-67, page 300
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