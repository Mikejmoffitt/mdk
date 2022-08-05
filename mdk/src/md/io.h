/* mdk gamepad support
Michael Moffitt 2018-2022 */
#ifndef MD_IO_H
#define MD_IO_H

#include <stdint.h>

// Serial port speed configurations
#define IO_BAUD_4800 0x00
#define IO_BAUD_2800 0x80
#define IO_BAUD_1200 0x40
#define IO_BAUD_300  0x20

typedef uint16_t MdButton;
// Button masks.
#define BTN_UP     0x0001
#define BTN_DOWN   0x0002
#define BTN_LEFT   0x0004
#define BTN_RIGHT  0x0008
#define BTN_B      0x0010
#define BTN_C      0x0020
#define BTN_A      0x0040
#define BTN_START  0x0080
#define BTN_Z      0x0100
#define BTN_Y      0x0200
#define BTN_X      0x0400
#define BTN_MODE   0x0800
// Special status bits.
#define MD_PAD_UNPLUGGED  0x4000  // This may also be set for an SMS controller.
#define MD_PAD_6B         0x8000

// Input state data for player 1 and 2. Test these variables with the BTN_*
// constants above to read button data.
extern uint16_t g_md_pad[2];       // Button data for the current frame.
extern uint16_t g_md_pad_pos[2];   // Buttons pressed on the current frame.
extern uint16_t g_md_pad_neg[2];   // Buttons released on the current frame.
extern uint16_t g_md_pad_prev[2];  // The state of buttons from the last frame.

// Configures control registers for gamepad reading.
// th interrupts are disabled for each port.
void md_io_init(void);

// Internal Use ---------------------------------------------------------------

// Poll controller inputs.
void md_io_poll(void);

// Generates g_md_pad_pos and g_md_pad_neg from g_md_pad.
void md_io_generate_edges(void);

#endif  // MD_IO_H
