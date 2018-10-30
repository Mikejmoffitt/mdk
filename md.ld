/* simple MD linker script

Based heavily on SGDK linker script */

OUTPUT_ARCH(m68k)
SEARCH_DIR(.)
__DYNAMIC = 0;

MEMORY
{
	rom :	ORIGIN = 0x00000000, LENGTH = 0x00400000
	ram :	ORIGIN = 0x00FF0000, LENGTH = 0x00010000
}

/* Configure stack to start at the top of memory
   borrowed from SGDK*/
PROVIDE (__stack = 0x01000000);

SECTIONS
{
	.text 0x00000000 :
	{
		KEEP(*(.text.keepboot)) *(.text .text.*)
		. = ALIGN(0x4);
		*(.rodata .rodata.*)

/*		. = ALIGN(0x4);
		__INIT_SECTION__ = . ;
		*(.ini)
		SHORT (0x4E75)

		__FINI_SECTION__ = . ;
		*(.fini)
		SHORT (0x4E75)*/

		_etext = .;
		*(.lit)
	} > rom

	_stext = SIZEOF (.text);

	.data 0x00FF0000 :
	AT ( ADDR (.text) + SIZEOF (.text) )
	{
/*		*(.got.plt) *(.got)*/
/*		*(.shdata)*/
		*(.data .data.*)
		_edata = .;
	} > ram
	_sdata = SIZEOF (.data);

	.bss 0x00FF0000 + SIZEOF (.data) :
	{
		_start = . ;
/*		*(.shbss)*/
		*(.bss .bss.*)
/*		*(COMMON)*/
		_bend = . ;
		end = . ;
	} > ram
	
	.stab 0 (NOLOAD) :
	{
		*(.stab)
	}
	
	.stabstr 0 (NOLOAD) :
	{
		*(.stabstr)
	}
	
	.eh_frame 0 (NOLOAD) :
	{
		*(.eh_frame)
	}

}
