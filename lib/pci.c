#include "pci.h"

#define NON_EXISTANT_DEVICE 0xFFFF

typedef struct device {
	uint32_t device_full_id;
	uint32_t device_pci_address;
} pci_device_t;

#define MAX_DEVICES_PER_BUS 32
static pci_device_t found_pci_devices[32]; // For now only bus 0

// A device is: vendor_id << 16 | device_ids
// field is the offset here: https://wiki.osdev.org/PCI
// Some bit manipulation is done so that non-aligned (i.e. not 4 byte offsets)
// still work Size is the number of bytes to write Ex. offset = 0x2, size = 2,
// write bytes 0x2, 0x3, does not work for more than 4 bytes unaligned, ex.
// offset = 0x3, size = 2 is undefined
// TODO document the implementation
void pci_write_field(uint32_t dev, uint8_t field, uint8_t size,
		     uint32_t value_to_write)
{
	uint32_t pci_addr = getPCIAddress(dev);
	uint32_t addr_to_read_write = pci_addr | field;
	// value_to_write is 1-4 bytes

	uint32_t aligned_addr_to_read_write = addr_to_read_write & ~0x3;
	uint8_t bytes_offset = addr_to_read_write & 0x3;

	outl(PCI_ADDR, aligned_addr_to_read_write);
	uint32_t tempValue = inl(PCI_DATA);

	// TODO, somehow remove the if?
	if (bytes_offset == 0) {
		tempValue &= (~bit_masks[size - 1] << (bytes_offset * 8));
	} else {
		tempValue &= (~bit_masks[size - 1] << (bytes_offset * 8)) |
			     bit_masks[bytes_offset -
				       1]; // Zero out the bytes we want to set
	}

	tempValue |= (value_to_write & bit_masks[size - 1])
		     << (bytes_offset * 8); // Set the bytes

	// kpanic_fmt("Writing 0x%x at 0x%x\n", tempValue,
	// aligned_addr_to_read_write);
	outl(PCI_DATA, tempValue); // Write out the data
}

uint32_t pci_read_field(uint32_t dev, uint8_t field, uint8_t size)
{
	uint32_t pci_addr = getPCIAddress(dev);
	uint32_t addr_to_read_write = pci_addr | field;
	// Field can be 0, 1 or 2 or 3 bytes offset

	outl(PCI_ADDR, addr_to_read_write & ~0x3);
	uint8_t bytes_offset = addr_to_read_write & 0x3;
	uint32_t value = inl(PCI_DATA);

	if (bytes_offset > (size - 1)) {
		kpanic_fmt("Error, pci read size bigger than the offset\n");
	}

	// Size can be 1, 2, 3, 4 bytes
	return (value >> (bytes_offset * 8)) & bit_masks[size - 1];
}

uint32_t getPCIAddress(uint32_t dev)
{
	uint32_t dev_address = 0xFFFFFFFF;
	// Do a lookup, TODO convert this to hashtable for O(1) performance
	for (uint8_t x = 0; x < MAX_DEVICES_PER_BUS; x++) {
		if (found_pci_devices[x].device_full_id == dev) {
			dev_address = found_pci_devices[x].device_pci_address;
			break;
		}
	}
	if (dev_address == 0xFFFFFFFF) {
		kpanic_fmt("ERROR device %x pci address not found\n", dev);
	}
	return dev_address;
}

// Slot = device number
uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t function,
			   uint8_t reg)
{
	uint32_t address_to_read = 0;
	uint32_t busl = (uint32_t)bus;
	uint32_t slotl = (uint32_t)slot;
	uint32_t functionl = (uint32_t)function;

	address_to_read |= 1U << 31; // Enable bit
	address_to_read |= busl << 16;
	address_to_read |= (slotl & 0x1F) << 11;
	address_to_read |= (functionl & 0x3) << 8;
	address_to_read |= (reg & 0xFC);

	outl(PCI_ADDR, address_to_read);

	return (inl(PCI_DATA) >> (reg & 0x2) * 8) & 0xFFFF;
}

uint32_t convertToPCIAddress(uint8_t bus, uint8_t slot, uint8_t function)
{
	uint32_t address_to_read = 0;
	uint32_t busl = (uint32_t)bus;
	uint32_t slotl = (uint32_t)slot;
	uint32_t functionl = (uint32_t)function;

	address_to_read |= 1U << 31; // Enable bit
	address_to_read |= busl << 16;
	address_to_read |= (slotl & 0x1F) << 11;
	address_to_read |= (functionl & 0x3) << 8;
	return address_to_read;
}

void pci_find_devices()
{
	uint8_t bus = 0;
	pci_device_t dev;

	for (uint8_t slot = 0; slot < MAX_DEVICES_PER_BUS; slot++) {
		uint16_t vendor = pciConfigReadWord(bus, slot, 0, 0);
		if (vendor != NON_EXISTANT_DEVICE) {
			uint16_t device_id = pciConfigReadWord(
				bus, slot, 0, 2); // High word of offset 0

			dev.device_full_id = (vendor << 16) | device_id;
			dev.device_pci_address =
				convertToPCIAddress(bus, slot, 0);

			found_pci_devices[slot] = dev;
		}
	}
}