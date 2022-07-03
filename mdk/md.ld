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

/* TODO: work banked SRAM into the ROM area for games that support saving. */
SECTIONS
{
	.text 0x00000000 :
	{
		KEEP(*(.text.keepboot)) *(.text .text.*)
		. = ALIGN(0x4);
		_etext = .;
		. = ALIGN(0x4);
		*(.rodata .rodata.*)
		. = ALIGN(0x4);
		*(.lit)
	} > rom

	_stext = SIZEOF (.text);

	.data 0x00FF0000 :
	AT ( ADDR (.text) + SIZEOF (.text) )
	{
		. = ALIGN(0x4);
		*(.data .data.*)
		. = ALIGN(0x4);
		_edata = .;
	} > ram
	_sdata = SIZEOF (.data);

	.bss 0x00FF0000 + SIZEOF (.data) :
	{
		. = ALIGN(0x4);
		_start = . ;
		*(.bss .bss.*)
		. = ALIGN(0x4);
		_bend = . ;
		end = . ;
	} > ram
}
