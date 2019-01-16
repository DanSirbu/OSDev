#include "e1000.h"
#include "kmalloc.h"
#include "serial.h"
#include "types.h"

// TODO, get these values using PCI
#define ETHERNET_IO_BASE 0xc03f
#define ETHERNET_MEM_BASE 0xFEBC0000
#define ETHERNET_IRQ_NUMBER 11

uint8_t mac[6];
uint32_t readCommand(uint16_t offset)
{
	return *((uint32_t *)(ETHERNET_MEM_BASE + offset));
}
void writeCommand(uint16_t addr, uint32_t value)
{
	uint32_t *ptr = (uint32_t *)(ETHERNET_MEM_BASE + addr);
	*ptr = value;
}
bool detectEEProm()
{
	uint32_t val = 0;
	writeCommand(REG_EEPROM, 0x1);
	bool eerprom_exists = false;

	for (int i = 0; i < 1000 && !eerprom_exists; i++) {
		val = readCommand(REG_EEPROM);
		if (val & 0x10)
			eerprom_exists = true;
		else
			eerprom_exists = false;
	}
	return eerprom_exists;
}
uint16_t eeprom_read(uint8_t addr)
{
	// bit 0 = tells hardware to start the read
	// 15-8 = read address
	writeCommand(REG_EEPROM, (((uint32_t)addr) << 8) | 1);

	// Bit 4 gets set to one when data received
	uint32_t eepromValue = readCommand(REG_EEPROM);
	while (!(eepromValue & (1 << 4)))
		; // Wait for data

	// Bit 31-16 contain the data
	return (uint16_t)((eepromValue >> 16) & 0xFFFF);
}
void readMacAddress()
{
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
// To decide on the best routing, routers use: IGMP, BGP
// IP gives packet to right computer
// TCP/UDP specify the program (port)+checksum

// Google works on concept of backlinks (how many people put a link to website,
// so it means it gets used and its not spam, (probably))

// Application layer -
// Presentation layer - Web servers, hyperlinks, pages, HTTP

// Session layer - Open Connection, PassInfo, close connection
// Transport layer - TCP/UDP
// Netowork layer - Switching/routing
// Data link layer - MAC
// Physical layer - physical bits
uint16_t pciCheckVendor(uint8_t bus, uint8_t slot);

// Temporary for now
void ethernet_main()
{
	// mem_base = pciConfigHeader->getPCIBar( PCI_BAR_MEM) & ~3;
	/*for(int i = 0; i < 10; i++) {
      uint16_t vendor1 = pciCheckVendor(0, i);
  }*/
	debug_print("detectEEProm %x\n", detectEEProm());
	readMacAddress();

	debug_print("Mac: 0x%x", (uint32_t)mac[0]);
	debug_print(" 0x%x", (uint32_t)mac[1]);
	debug_print(" 0x%x", (uint32_t)mac[2]);
	debug_print(" 0x%x", (uint32_t)mac[3]);
	debug_print(" 0x%x", (uint32_t)mac[4]);
	debug_print(" 0x%x\n", (uint32_t)mac[5]);

	// Set link up
	uint32_t rctl_val = readCommand(REG_CTRL);
	writeCommand(REG_CTRL, rctl_val | ECTRL_SLU);

	// Clear multicast filter
	// Apparently needed because it breaks stuff
	// https://github.com/blanham/ChickenOS/blob/master/src/device/net/e1000.c ,
	// line 255
	for (int i = 0; i < 0x80; i++) {
		writeCommand(0x5200 + i * 4,
			     0); // Multicast array table, page 327
	}
	uint32_t RAL = readCommand(0x5400);
	uint32_t RAH = readCommand(0x5404);
	debug_print("RAL: 0x%x\n", RAL);

	// Enable interrupts
	// TODO??? what is this
	writeCommand(REG_IMASK, 0x1F6DC); // Page 297, 13.4.20
	writeCommand(
		REG_IMASK,
		0xff & ~4); // TODO WHY SET IT AGAIN to enable different interrupts?
	uint32_t cause =
		readCommand(0xc0); // Page 293, Interrupt Cause Read register
	// debug_print("E1000, interrupts init: 0x%x\n", cause);
	rxinit();
	txinit();

	debug_print("E1000 card initialized\n");

	uint32_t rctrl_flags = readCommand(REG_RCTRL);
	writeCommand(REG_RCTRL, rctrl_flags | RCTL_EN); // Enable receive
}
uint8_t rx_cur, tx_cur;
struct e1000_rx_desc *rx_descs[E1000_NUM_RX_DESC];
struct e1000_tx_desc *tx_descs[E1000_NUM_RX_DESC];

void rxinit()
{
	// NIC works with physical addresses
	void *ptr;
	struct e1000_rx_desc *descs;
	// Allocate space for receive descriptors info
	ptr = kmalloc_align(sizeof(struct e1000_rx_desc) * E1000_NUM_RX_DESC +
				    16,
			    16); // Why + 16?

	descs = (struct e1000_rx_desc *)ptr;
	for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
		rx_descs[i] = (struct e1000_rx_desc *)descs + i;
		rx_descs[i]->addr_low = (uint32_t)kmalloc_align(
			8192 + 16,
			16); // malloc TODO, check why additional 16 bytes are needed
		rx_descs[i]->addr_high = 0;
		rx_descs[i]->status = 0; // RDesc.status Table3-2, page 21
	}
	// Give card the pointer to descriptor
	// Set up the receive descriptor layout base pointer
	// Note 4 lowest bits are ignored, so it must be 16 bytes aligned ex. 0xF0,
	// TODO check
	writeCommand(REG_RXDESCLO, (size_t)ptr); // Low 32 bits, page 306
	writeCommand(REG_RXDESCHI, 0); // High 32 bits

	// Give card the total length of descriptors
	// page 307
	// Receive buffer/descriptors length in bytes (sizeof(e1000_rx_desc) = 16)
	writeCommand(REG_RXDESCLEN, E1000_NUM_RX_DESC * 16);

	// Setup head and tail pointers to descriptors 0 and the last one
	// TODO why?
	writeCommand(REG_RXDESCHEAD, 0);
	writeCommand(REG_RXDESCTAIL, E1000_NUM_RX_DESC);

	// Initialize receive descriptor index (that we will use to get the data)
	rx_cur = 0;

	// RCTL_BSIZE_8192 = set the receive buffer size, which is 8192
	// RTCL_RDMTS_HALF =  ICR.RXDMT0 interrupt fired when half of the receive
	// descriptors are used RCTL_BAM accept broadcast packets RCTL_SECRC strip CRC
	// RCTL_UPE | RCTL_MPE Unicast Promiscuous Enabled, Multicast Promiscuous
	// Enabled

	// RCTL_EN | ?? where is it
	uint32_t rctl_params =
		(2 << 16) | (1 << 25) | (1 << 26) | (1 << 15) | (1 << 5) |
		(0 << 8) | (0 << 4) | (0 << 3) |
		RCTL_SBP; // RCTL_EN | RCTL_SBP | RCTL_UPE | RCTL_MPE | RTCL_RDMTS_QUARTER
	// | RCTL_LPE | RCTL_BAM | RCTL_SECRC | RCTL_BSIZE_8192;
	writeCommand(REG_RCTRL, rctl_params); // Table 13-67, page 300
}
void txinit()
{
	// NIC works with physical addresses
	void *ptr;
	struct e1000_tx_desc *descs;
	// Allocate space for receive descriptors info
	ptr = kmalloc_align(sizeof(struct e1000_tx_desc) * E1000_NUM_TX_DESC +
				    16,
			    16); // Why + 16?

	descs = (struct e1000_tx_desc *)ptr;
	for (int i = 0; i < E1000_NUM_TX_DESC; i++) {
		tx_descs[i] = (struct e1000_tx_desc *)descs + i;
		tx_descs[i]->addr_low = 0;
		tx_descs[i]->addr_high = 0;
		tx_descs[i]->cmd = 0; // RDesc.status Table3-2, page 21
	}
	// Give card the pointer to descriptor
	// Set up the receive descriptor layout base pointer
	// Note 4 lowest bits are ignored, so it must be 16 bytes aligned ex. 0xF0,
	// TODO check
	writeCommand(REG_TXDESCLO, (size_t)ptr); // Low 32 bits, page 306
	writeCommand(REG_TXDESCHI, 0); // High 32 bits

	// Give card the total length of descriptors
	// page 307
	// Receive buffer/descriptors length in bytes (sizeof(e1000_rx_desc) = 16)
	writeCommand(REG_TXDESCLEN, E1000_NUM_TX_DESC * 16);

	// Setup head and tail pointers to descriptors 0 and the last one
	// TODO why?
	writeCommand(REG_TXDESCHEAD, 0);
	writeCommand(REG_TXDESCTAIL, E1000_NUM_TX_DESC);

	// Initialize receive descriptor index (that we will use to get the data)
	tx_cur = 0;

	// RCTL_EN
	writeCommand(REG_TCTRL, TCTL_EN | TCTL_PSP); // page 311
}
void E1000_Interrupt()
{
	writeCommand(REG_IMASK, 0x1); // Aknowledge interrupt received
	uint32_t interrupt_cause = readCommand(
		0xc0); // Interrupt cause register, set when an interrupt occurs, p 293
	// 0x4 = link status changed
	// rx();
	// 0x2 = Transmit Queue Empty

	debug_print("E1000 interrupt cause 0x%x\n", interrupt_cause);
}
void rx()
{
	uint16_t old_cur;

	while (rx_descs[rx_cur]->status & 0x1) {
	}
}