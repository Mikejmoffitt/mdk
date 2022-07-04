// mdk memory map and MMIO definitions
// 2018-2022 Michael Moffitt
// ===========================================================================
#ifndef MD_MMIO_H
#define MD_MMIO_H

// -----------------------------------------------------------------------------
// I/O (MD Pads)
// -----------------------------------------------------------------------------

// Port definitions
#define IO_LOC_BASE    (0xA10000)
#define IO_LOC_VERSION (IO_LOC_BASE + 0x01)
#define IO_LOC_DATA1   (IO_LOC_BASE + 0x03)
#define IO_LOC_DATA2   (IO_LOC_BASE + 0x05)
#define IO_LOC_DATA3   (IO_LOC_BASE + 0x07)
#define IO_LOC_CTRL1   (IO_LOC_BASE + 0x09)
#define IO_LOC_CTRL2   (IO_LOC_BASE + 0x0B)
#define IO_LOC_CTRL3   (IO_LOC_BASE + 0x0D)
#define IO_LOC_TXD1    (IO_LOC_BASE + 0x0F)
#define IO_LOC_RXD1    (IO_LOC_BASE + 0x11)
#define IO_LOC_SERIO1  (IO_LOC_BASE + 0x13)
#define IO_LOC_TXD2    (IO_LOC_BASE + 0x15)
#define IO_LOC_RXD2    (IO_LOC_BASE + 0x17)
#define IO_LOC_SERIO2  (IO_LOC_BASE + 0x19)
#define IO_LOC_TXD3    (IO_LOC_BASE + 0x1B)
#define IO_LOC_RXD3    (IO_LOC_BASE + 0x1D)
#define IO_LOC_SERIO3  (IO_LOC_BASE + 0x1F)

// -----------------------------------------------------------------------------
// I/O (System C / C2)
// -----------------------------------------------------------------------------

#define IO_SYSTEMC_LOC_BASE  (0x840000)
#define IO_SYSTEMC_LOC_PORTA (IO_SYSTEMC_LOC_BASE + 0x01)
#define IO_SYSTEMC_LOC_PORTB (IO_SYSTEMC_LOC_BASE + 0x03)
#define IO_SYSTEMC_LOC_PORTC (IO_SYSTEMC_LOC_BASE + 0x05)
#define IO_SYSTEMC_LOC_PORTD (IO_SYSTEMC_LOC_BASE + 0x07)
#define IO_SYSTEMC_LOC_PORTE (IO_SYSTEMC_LOC_BASE + 0x09)
#define IO_SYSTEMC_LOC_PORTF (IO_SYSTEMC_LOC_BASE + 0x0B)
#define IO_SYSTEMC_LOC_PORTG (IO_SYSTEMC_LOC_BASE + 0x0D)
#define IO_SYSTEMC_LOC_PORTH (IO_SYSTEMC_LOC_BASE + 0x0F)
#define IO_SYSTEMC_LOC_PROT0 (IO_SYSTEMC_LOC_BASE + 0x11)
#define IO_SYSTEMC_LOC_PROT1 (IO_SYSTEMC_LOC_BASE + 0x13)
#define IO_SYSTEMC_LOC_PROT2 (IO_SYSTEMC_LOC_BASE + 0x15)
#define IO_SYSTEMC_LOC_PROT3 (IO_SYSTEMC_LOC_BASE + 0x17)
#define IO_SYSTEMC_LOC_CTRL0 (IO_SYSTEMC_LOC_BASE + 0x19)
#define IO_SYSTEMC_LOC_CTRL1 (IO_SYSTEMC_LOC_BASE + 0x1B)
#define IO_SYSTEMC_LOC_CTRL2 (IO_SYSTEMC_LOC_BASE + 0x1D)
#define IO_SYSTEMC_LOC_CTRL3 (IO_SYSTEMC_LOC_BASE + 0x1F)

#define IO_SYSTEMC_LOC_VCTRL (IO_SYSTEMC_LOC_BASE + 0x201)

// -----------------------------------------------------------------------------
// I/O (Sys)
// -----------------------------------------------------------------------------

#define SYS_PORT_VERSION   (*(volatile uint8_t *)(IO_LOC_BASE + 1))
#define SYS_Z80_PRG_LOC    (0xA00000)
#define SYS_Z80_PORT_BUS   (*(volatile uint16_t *)0xA11100)
#define SYS_Z80_PORT_RESET (*(volatile uint16_t *)0xA11200)

// -----------------------------------------------------------------------------
// CRAM (System C / C2)
// -----------------------------------------------------------------------------

#define CRAM_SYSTEMC_LOC_BASE (0x8C0000)

// -----------------------------------------------------------------------------
// VDP
// -----------------------------------------------------------------------------

#define VDP_LOC_BASE    (0xC00000)
#define VDPPORT_DATA    (*(volatile uint16_t*)(VDP_LOC_BASE))
#define VDPPORT_CTRL    (*(volatile uint16_t*)(VDP_LOC_BASE + 4))
#define VDPPORT_CTRL32  (*(volatile uint32_t*)(VDP_LOC_BASE + 4))
#define VDPPORT_HVCOUNT (*(volatile uint16_t*)(VDP_LOC_BASE + 8))



#endif  // MD_MMIO_H
