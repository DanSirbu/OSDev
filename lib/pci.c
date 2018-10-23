#include "../include/pci.h"

#define NON_EXISTANT_DEVICE 0xFFFF

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

    outl(PCI_ADDR, address_to_read);

    return (inl(PCI_DATA) >> (reg & 0x2) * 8) & 0xFFFF;
}

uint16_t pci_get_devices(uint8_t bus, uint8_t slot) {
    uint16_t vendor;//, device;
    vendor = pciConfigReadWord(bus, slot, 0, 0);
    if(vendor != NON_EXISTANT_DEVICE) {
        uint16_t temp;
        //= pciConfigReadWord(bus, slot, 0, 2); //High word of offset 0

    }
    return vendor;
}