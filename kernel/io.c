#include "io.h"

uint8_t inb(uint16_t p_port)
{
	uint8_t l_ret;
	__asm__ volatile("inb %1, %0" : "=a"(l_ret) : "dN"(p_port));
	return l_ret;
}
void outb(uint16_t p_port, uint8_t p_data)
{
	__asm__ volatile("outb %1, %0" : : "dN"(p_port), "a"(p_data));
}

uint16_t inw(uint16_t p_port)
{
	uint16_t l_ret;
	__asm__ volatile("inw %1, %0" : "=a"(l_ret) : "dN"(p_port));
	return l_ret;
}
void outw(uint16_t p_port, uint16_t p_data)
{
	__asm__ volatile("outw %1, %0" : : "dN"(p_port), "a"(p_data));
}

uint32_t inl(uint16_t p_port)
{
	uint32_t l_ret;
	__asm__ volatile("inl %1, %0" : "=a"(l_ret) : "dN"(p_port));
	return l_ret;
}
void outl(uint16_t p_port, uint32_t p_data)
{
	__asm__ volatile("outl %1, %0" : : "dN"(p_port), "a"(p_data));
}