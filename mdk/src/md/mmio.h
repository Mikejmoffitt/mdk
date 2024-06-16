// mdk memory map and MMIO definitions
// 2018-2024 Michael Moffitt
#pragma once

#define ROM_BASE (0x00000000)
#define WRAM_BASE (0xFFFF0000)
#define WRAM_SIZE (0x10000)
#define VRAM_SIZE (0x10000)

#define SRAM_BASE (0x200000)
#define SRAM_CTRL (0xA130F1)

/* I/O (MD Pads) */
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
/* I/O (Sys) */
#define SYS_Z80_PRG_LOC        (0xA00000)
#define SYS_Z80_PORT_BUS_LOC   (0xA11100)
#define SYS_Z80_PORT_RESET_LOC (0xA11200)
#define TMSS_PORT    (0xA14000)
/* VDP */
#define VDP_LOC_BASE      (0xC00000)
/* PSG */
#define PSG_LOC_BASE (0xC00011)
/* System C/C2 additions */
/* Color RAM */
#define CRAM_SYSTEMC_LOC_BASE (0x8C0000)
/* I/O ports (buttons, some peripheral control) */
#define SYSC_IO_LOC_BASE  (0x840000)
#define SYSC_IO_LOC_PORTA (SYSC_IO_LOC_BASE + 0x01)
#define SYSC_IO_LOC_PORTB (SYSC_IO_LOC_BASE + 0x03)
#define SYSC_IO_LOC_PORTC (SYSC_IO_LOC_BASE + 0x05)
#define SYSC_IO_LOC_PORTD (SYSC_IO_LOC_BASE + 0x07)
#define SYSC_IO_LOC_PORTE (SYSC_IO_LOC_BASE + 0x09)
#define SYSC_IO_LOC_PORTF (SYSC_IO_LOC_BASE + 0x0B)
#define SYSC_IO_LOC_PORTG (SYSC_IO_LOC_BASE + 0x0D)
#define SYSC_IO_LOC_PORTH (SYSC_IO_LOC_BASE + 0x0F)
/* I/O protection registers (for reading the string "SEGA") */
#define SYSC_IO_LOC_PROT0 (SYSC_IO_LOC_BASE + 0x11)
#define SYSC_IO_LOC_PROT1 (SYSC_IO_LOC_BASE + 0x13)
#define SYSC_IO_LOC_PROT2 (SYSC_IO_LOC_BASE + 0x15)
#define SYSC_IO_LOC_PROT3 (SYSC_IO_LOC_BASE + 0x17)
/* I/O control registers */
#define SYSC_IO_LOC_CTRL0 (SYSC_IO_LOC_BASE + 0x19)
#define SYSC_IO_LOC_CTRL1 (SYSC_IO_LOC_BASE + 0x1B)
#define SYSC_IO_LOC_CTRL2 (SYSC_IO_LOC_BASE + 0x1D)
#define SYSC_IO_LOC_CTRL3 (SYSC_IO_LOC_BASE + 0x1F)
/* Protection register */
#define SYSC_PROTECTION_LOC_BASE     (0x800000)
#define SYSC_PROTECTION_LOC_SECURITY (SYSC_PROTECTION_LOC_BASE + 0x001)
#define SYSC_PROTECTION_LOC_VCTRL    (SYSC_PROTECTION_LOC_BASE + 0x201)

#define SYSC_IO_UPD7759_LOC_BASE (0x880000)
#define SYSC_IO_UPD7759_PHRASE_LOC (SYSC_IO_UPD7759_LOC_BASE + 1)

#ifndef __ASSEMBLY__
#define PSG_PORT          (*(volatile uint8_t *)(PSG_LOC_BASE))
#define SYS_PORT_VERSION  (*(volatile uint8_t *)(IO_LOC_BASE + 1))
#define VDPPORT_DATA      (*(volatile uint16_t*)(VDP_LOC_BASE))
#define VDPPORT_DATA32    (*(volatile uint32_t*)(VDP_LOC_BASE))
#define VDPPORT_CTRL      (*(volatile uint16_t*)(VDP_LOC_BASE + 4))
#define VDPPORT_CTRL32    (*(volatile uint32_t*)(VDP_LOC_BASE + 4))
#define VDPPORT_HVCOUNT   (*(volatile uint16_t*)(VDP_LOC_BASE + 8))
#define VDPPORT_DBG_SEL   (*(volatile uint16_t *)(VDP_LOC_BASE + 0x18))
#define VDPPORT_DBG_DATA  (*(volatile uint16_t *)(VDP_LOC_BASE + 0x1C))
#endif  /* __ASSEMBLY__ */
