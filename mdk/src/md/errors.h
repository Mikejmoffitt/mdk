#pragma once

// Macro forfailing out to an error
#ifdef __ASSEMBLER__
.macro	showerr msgid
	movem.l	d0-d7/a0-a7, -(sp)
	moveq	#\msgid, d0
	jmp	(md_error_dispatch)
.endm
#endif


#define MD_ERRMSG_BUS_ERROR             0
#define MD_ERRMSG_ADDRESS_ERROR         1
#define MD_ERRMSG_ILLEGAL_INSTRUCTION   2
#define MD_ERRMSG_DIV_ZERO              3
#define MD_ERRMSG_CHK                   4
#define MD_ERRMSG_TRAPV                 5
#define MD_ERRMSG_PRIVELEGE             6
#define MD_ERRMSG_TRACE                 7
#define MD_ERRMSG_UNUSED_IRQ            8
#define MD_ERRMSG_ALINE_EMU             9
#define MD_ERRMSG_FLINE_EMU            10
#define MD_ERRMSG_RESERVED             11
#define MD_ERRMSG_COPROC_VIOLATION     12
#define MD_ERRMSG_FORMAT               13
#define MD_ERRMSG_UNINIT               14
#define MD_ERRMSG_SPURIOUS             15
#define MD_ERRMSG_TRAP0X0              16
#define MD_ERRMSG_TRAP0X1              17
#define MD_ERRMSG_TRAP0X2              18
#define MD_ERRMSG_TRAP0X3              19
#define MD_ERRMSG_TRAP0X4              20
#define MD_ERRMSG_TRAP0X5              21
#define MD_ERRMSG_TRAP0X6              22
#define MD_ERRMSG_TRAP0X7              23
#define MD_ERRMSG_TRAP0X8              24
#define MD_ERRMSG_TRAP0X9              25
#define MD_ERRMSG_TRAP0XA              26
#define MD_ERRMSG_TRAP0XB              27
#define MD_ERRMSG_TRAP0XC              28
#define MD_ERRMSG_TRAP0XD              29
#define MD_ERRMSG_TRAP0XE              30
#define MD_ERRMSG_TRAP0XF              31
#define MD_ERRMSG_UNIMP                32
#define MD_ERRMSG_IRQ1                 33
#define MD_ERRMSG_IRQ2                 34
#define MD_ERRMSG_IRQ3                 35
#define MD_ERRMSG_IRQ4                 36
#define MD_ERRMSG_IRQ5                 37
#define MD_ERRMSG_IRQ6                 38
#define MD_ERRMSG_IRQ7                 39
