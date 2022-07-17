/* mdk IRQ and exception handler registration
Michael Moffitt 2018-2022

 */
#ifndef MD_IRQ_H
#define MD_IRQ_H

typedef enum MdIrqType
{
	MD_IRQ_VBLANK = 0,  // Vertical raster interrupt.
	MD_IRQ_HBLANK = 1,  // Horizontal raster interrupt.
	MD_IRQ_IO     = 2,  // TH pin transitions for controller port.
} MdIrqType;

// Registers a handler for an interrupt type. NULL may be passed to remove any
// pre-existing handler.
// If you want to register a function that won't clobber d0-d1/a0-a1, and want
// a little speed boost (good for HBL handlers) use md_irq_register_unsafe().
void md_irq_register(MdIrqType type, void (*function_ptr)(void));

// Same as md_irq_register(), but the function will be called without register
// clobber protection. Unless you carefully wrote this function in asm, use
// md_irq_register()!
void md_irq_register_unsafe(MdIrqType type, void (*function_ptr)(void));

#endif  // MD_IRQ_H
