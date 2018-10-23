#pragma once
#include "types.h"
#include "io.h"

#define PCI_ADDR 0xCF8
#define PCI_DATA 0xCFC

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

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t function, uint8_t reg);