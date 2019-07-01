#pragma once
#include "sys/types.h"

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t data);

uint16_t inw(uint16_t p_port);
void outw(uint16_t p_port, uint16_t p_data);

uint32_t inl(uint16_t p_port);
void outl(uint16_t p_port, uint32_t p_data);