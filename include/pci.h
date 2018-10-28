#pragma once
#include "io.h"
#include "serial.h"
#include "types.h"

#define PCI_ADDR 0xCF8
#define PCI_DATA 0xCFC

#define PCI_CMD_REG 0x4
#define PCI_CMD_SIZE 2

typedef struct {
	uint16_t vendor_id;
	uint16_t dev_id;
	uint16_t command;
	uint16_t status;
	uint8_t revision_id;
	uint8_t prog_if;
	uint8_t subclass;
	uint8_t class_code;
	uint8_t cache_line_size;
	uint8_t latency_timer;
	uint8_t header_type;
	uint8_t BIST;

	uint32_t BAR0;
	uint32_t BAR1;
	uint32_t BAR2;
	uint32_t BAR3;
	uint32_t BAR4;
	uint32_t BAR5;

	uint32_t cardbus_cis_pointer;
	uint16_t subsystem_vendor_id;
	uint16_t subsystem_id;
	uint32_t rom_base_address;
	uint8_t capabilities_ptr;

	uint8_t reserved1;
	uint16_t reserved2;
	uint32_t reserved3;

	uint8_t interrupt_line;
	uint8_t interrupt_pin;
	uint8_t min_grant;
	uint8_t max_latency;
} __attribute__((packed)) pci_dev_t;

void pci_find_devices();
void pci_write_field(uint32_t dev, uint8_t field, uint8_t size,
		     uint32_t value_to_write);
uint32_t pci_read_field(uint32_t dev, uint8_t field, uint8_t size);
uint32_t getPCIAddress(uint32_t dev);

static const uint32_t bit_masks[] = { 0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF };

// clang-format off
#define pci_get_device(vendor_id, device_id) ((uint32_t)(vendor_id << 16 | device_id))
// clang-format on