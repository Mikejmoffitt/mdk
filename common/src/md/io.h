/* md-toolchain I/O peripheral support
Michael Moffitt 2018 */
#ifndef IO_H
#define IO_H

#include <stdint.h>

// TODO: Make MdIoMode type, configure IO ports using it.
// TODO: Serial support
// TODO: 6-button controllers

// Button masks
typedef enum MdButton
{
	BTN_UP    = 0x0001,
	BTN_DOWN  = 0x0002,
	BTN_LEFT  = 0x0004,
	BTN_RIGHT = 0x0008,
	BTN_B     = 0x0010,
	BTN_C     = 0x0020,
	BTN_A     = 0x0040,
	BTN_START = 0x0080,
	BTN_X     = 0x0100,
	BTN_Y     = 0x0200,
	BTN_Z     = 0x0400,
	BTN_MODE  = 0x0800,
} MdButton;

// Serial port speed configurations
#define IO_BAUD_4800 0x00
#define IO_BAUD_2800 0x80
#define IO_BAUD_1200 0x40
#define IO_BAUD_300  0x20

// Port definitions
#define IO_LOC_BASE    0xA10000
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

// Poll controller inputs.
// This should be called during vblank.
void io_poll(void);

// Get the state of one control port (0 through 2)
// This returns from the cache captured during vblank, so there are no
// concerns about touching IO ports or disabling the Z80.
MdButton io_pad_read(uint8_t port);

// Configures a port's control register for gamepad reading.
// th interrupts are disabled for each port.
void io_gamepad_en(uint8_t port);

// Turn on IRQ generation for the TH pin.
// Make sure to enable thint on the VDP (vdp_set_thint_en)
void io_thint_en(uint8_t port, uint8_t enabled);

#endif // IO_H
