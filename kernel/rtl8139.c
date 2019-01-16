// https://wiki.osdev.org/RTL8139
#include "rtl8139.h"
#include "pci.h"
#include "serial.h"
#include "trap.h"

#define RLT_VENDOR_ID 0x10EC
#define RTL_DEVICE_ID 0x8139

#define RTL_DEVICE pci_get_device(RLT_VENDOR_ID, RTL_DEVICE_ID)

#define RTL_IO_BASE 0xc000
// IRQ 11

#define REG_CONFIG_1 0x52
#define REG_IMR 0x3C
#define REG_ISR 0x3E

void RTL8139_Interrupt()
{
	uint16_t status = inw(RTL_IO_BASE + REG_ISR);

	outw(RTL_IO_BASE + REG_ISR,
	     0x1); // Aknowledge the interrupt, NOTE: reading
		// isn't enough, the documentation is wrong

	debug_print("Status %x\n", status);
}

size_t *rx_buffer;
#define CMD_RX_EN (1 << 3)
#define CMD_TX_EN (1 << 2)

void RTL8139_Init()
{
	//Register Interrupt Handler
	register_handler(32 + 11, RTL8139_Interrupt);

	// Enable PCI Bus Mastering
	uint32_t pci_command_reg =
		pci_read_field(RTL_DEVICE, PCI_CMD_REG, PCI_CMD_SIZE);
	pci_command_reg |= 1 << 2;
	pci_write_field(RTL_DEVICE, PCI_CMD_REG, PCI_CMD_SIZE, pci_command_reg);

	outb(RTL_IO_BASE + REG_CONFIG_1, 0x0); // Power on device

	// Reset RX and TX buffers
	outb(RTL_IO_BASE + 0x37, 0x10);
	while ((inb(RTL_IO_BASE + 0x37) & 0x10) != 0)
		; // Wait for reset to occur

	// Init receive buffer
	rx_buffer = kmalloc_align(8192 + 16 + 1500, 16);
	outl(RTL_IO_BASE + 0x30, (size_t)rx_buffer);

	// Interrupt Mask Register IMR (1 = enabled)
	// Interrupt Service Register (ISR)

	outw(RTL_IO_BASE + REG_IMR,
	     0x0005); // Enable the Transmit OK (TOK) and Receive Ok (ROK) interrupts

	// Tell RTL8139 to accept packets
	outl(RTL_IO_BASE + 0x44,
	     0xf | (1 << 7)); // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP

	// Enable receive and transmit
	// Sets the RE and TE bits high
	// TODO ENABLE Transmit
	outb(RTL_IO_BASE + 0x37, CMD_RX_EN);
}
