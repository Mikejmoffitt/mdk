#include "md/irq.h"
#include "md/sys.h"

#include <stdint.h>
#include <stdlib.h>

void (*g_irq_h_func)(void) = NULL;
void (*g_irq_io_func)(void) = NULL;
void (*g_irq_v_func)(void) = NULL;

#define IRQ_NO_CLOBBER_FLAG_MASK 0x80000000

// Registers a handler for an interrupt type. NULL may be passed to remove any
// pre-existing handler.
void md_irq_register(MdIrqType type, void (*function_ptr)(void))
{
	const uint16_t ints_were_enabled = md_sys_get_ints_enabled();
	md_sys_di();
	switch (type)
	{
		default:
			break;
		case MD_IRQ_VBLANK:
			g_irq_v_func = function_ptr;
			break;
		case MD_IRQ_IO:
			g_irq_io_func = function_ptr;
			break;
		case MD_IRQ_HBLANK:
			g_irq_h_func = function_ptr;
			break;
	}

	if (ints_were_enabled) md_sys_ei();
}

void md_irq_register_unsafe(MdIrqType type, void (*function_ptr)(void))
{
	// Don't set the magic no-clobber bit if the intent is to unregister.
	if (function_ptr != NULL)
	{
		uint32_t *ptr_as_uint32 = (uint32_t *)function_ptr;
		*ptr_as_uint32 |= IRQ_NO_CLOBBER_FLAG_MASK;
	}
	md_irq_register(type, function_ptr);
}
