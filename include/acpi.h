#pragma once
#include "types.h"
#include "serial.h"

typedef struct RSDPDescriptor {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RsdtAddress;
} __attribute__ ((packed)) RSDPDescriptor_t;

typedef struct RSDPDescriptor20 {
 RSDPDescriptor_t firstPart;
 
 uint32_t Length;
 uint32_t XsdtAddress_low;
 uint32_t XsdtAddress_high;
 uint8_t ExtendedChecksum;
 uint8_t reserved[3];
} __attribute__ ((packed)) RSDPDescriptor20_t;

typedef struct ACPISDTHeader {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
} __attribute__ ((packed)) ACPISDTHeader_t;


void acpi_init();
void acpi_10_init(RSDPDescriptor_t* rsdp_10);