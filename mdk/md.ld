OUTPUT_ARCH(m68k)
SEARCH_DIR(.)
__DYNAMIC = 0;

MEMORY
{
	rom :	ORIGIN = 0x00000000, LENGTH = 0x00400000
	ram :	ORIGIN = 0xFFFF0000, LENGTH = 0x00010000
}

PROVIDE (__stack = 0x00000000);

/* TODO: work banked SRAM into the ROM area for games that support saving. */
SECTIONS
{
	.text 0x00000000 :
	{
		KEEP(*(.text.keepboot)) *(.text .text.*)
		. = ALIGN(0x2);
		_etext = .;
		. = ALIGN(0x2);
		*(.rodata .rodata.*)
		. = ALIGN(0x2);
		*(.lit)
	} > rom

	_stext = SIZEOF (.text);

	.data 0xFFFF0000 :
	AT ( ADDR (.text) + SIZEOF (.text) )
	{
		. = ALIGN(0x2);
		*(.data .data.*)
		. = ALIGN(0x2);
		_edata = .;
	} > ram
	_sdata = SIZEOF (.data);

	.bss 0xFFFF0000 + SIZEOF (.data) :
	{
		. = ALIGN(0x2);
		_start = . ;
		*(.bss .bss.*)
		. = ALIGN(0x2);
		_bend = . ;
		end = . ;
	} > ram
}
