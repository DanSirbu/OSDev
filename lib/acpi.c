#include "../include/acpi.h"

#define RSDP_BASE 0xc00f5b80 // Hardcoded for now

void acpi_init() {
    RSDPDescriptor20_t *rsdp = (RSDPDescriptor20_t*) RSDP_BASE;
    if(rsdp->firstPart.Revision == 0) { //ACPI 1.0
        kpanic_fmt("Using ACPI 1.0\n");
    } else {
        kpanic_fmt("Using ACPI 2.0\n");
    }

    //Check if ACPI 1.0 is valid
    //Add up all the bytes in the firstpart and if the total is zero, it's valid
    uint32_t total = 0;
    for(uint8_t *rsdp_checksum = (uint8_t*) RSDP_BASE; rsdp_checksum < (((uint8_t*) RSDP_BASE) + sizeof(RSDPDescriptor_t)); rsdp_checksum += 1) {
        total += *rsdp_checksum;
    }
    if((total & 0xFF) != 0) {
        kpanic_fmt("ACPI 1.0 invalid, total is 0x%x\n", total);
        return;
    }

    //Check if ACPI > 1.0 is valid
    if(rsdp->firstPart.Revision != 0) {
        total = 0;
        for(uint8_t *rsdp_checksum1 = (uint8_t*) RSDP_BASE + sizeof(RSDPDescriptor_t); rsdp_checksum1 < ((uint8_t*) RSDP_BASE) + sizeof(RSDPDescriptor20_t); rsdp_checksum1 += 1) {
            total += *rsdp_checksum1;
        }
        if((total & 0xFF) != 0) {
            kpanic_fmt("ACPI 2.0 invalid, total is 0x%x\n", total);
            return;
        }
    }
    if(rsdp->firstPart.Revision == 0) {
        acpi_10_init((RSDPDescriptor_t*) rsdp);
    }
    else {
        kpanic_fmt("TODO: Initialize ACPI 2.0\n");
    }
}

void acpi_10_init(RSDPDescriptor_t* rsdp_10) {
    kpanic_fmt("RSDT addr: 0x%x\n", rsdp_10->RsdtAddress);
    ACPISDTHeader_t *rsdt_table_header = (ACPISDTHeader_t*) (rsdp_10->RsdtAddress + 0xC0000000); //convert to virtual address
}