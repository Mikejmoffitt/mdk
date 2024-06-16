// mdk IRQ and exception handler registration
// Michael Moffitt 2018-2024
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

typedef enum MdIrqType
{
	MD_IRQ_VBLANK = 0,  // Vertical raster interrupt.
	MD_IRQ_HBLANK = 1,  // Horizontal raster interrupt.
	MD_IRQ_IO     = 2,  // TH pin transitions for controller port.
} MdIrqType;

// Registers a handler for an interrupt type. NULL may be passed to remove any
// pre-existing handler.
void md_irq_register(MdIrqType type, void (*function_ptr)(void));

#ifdef __cplusplus
}
#endif  // __cplusplus
