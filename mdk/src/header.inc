/* Sega Megadrive / Genesis header

The header provides metadata to the system and some emulators. At minimum, it
must begin with "SEGA" for TMSS-equipped systems to run it. In addition, at
times the extra memory data is used to determine whether or not save data
will be stored.

Do be sure that the fields remain at their correct length. Extraneous
characters will be truncated. A legend has been placed placed above every
.ascii statement to give a visual reference for the correct length. */

/* Console name - must begin with SEGA.*/
	.org	0x000100
	/* 16:   ________________ */
	.ascii	"SEGA MEGA DRIVE "
/* Copyright / author information. Sixteen characters. */
	.org	0x000110
	/* 16:   ________________ */
	.ascii	"                "

/* Game name (Japanese/Domestic). 48 characters. */
	.org	0x000120
	/* 48:   ________________________________________________ */
	.ascii	"                                                "

/* Game name (Overseas/World). 48 characters. */
	.org	0x000150
	/* 48:   ________________________________________________ */
	.ascii	"                                                "

/* Game serial number. Sixteen characters. */
	.org	0x000180
	/* Serial number */
	/* 16:   _______________*/
	.ascii	"GM 00000000-00"

/* Checksum (2 bytes) */
	.org	0x00018E
	.short	0x0000

/* I/O Support */
	/* 16:   ________________ */
	.org	0x000190
	.ascii	"JD              "

/* ROM start and end */
	.org	0x0001A0
	.long	0
	.long	0x001FFFFF

/* Work RAM start and end */
	.org	0x0001A8
	.long	0x000FF000
	.long	0x000FFFFF

/* Backup memory */
	.org	0x0001B0
	.ascii	"RA"
	.byte	0xF8
	.byte	0x20

/* Backup RAM start and end*/
	.long	0x200001
	.long	0x20FFFF

/* Modem */
	.org	0x0001BC
	/* 12:   ____________ */
	.ascii	"            "

/* Reserved */
	.org	0x0001C8
	/* 40:   ________________________________________ */
	.ascii	"                                        "

/* Country codes */
	.org	0x0001F0
	.ascii	"JUE"

/* Reserved */
	.org	0x0001F3
	/* 13:   _____________ */
	.ascii	"             "

