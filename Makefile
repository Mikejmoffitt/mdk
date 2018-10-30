# Configuration
CPUTYPE := 68000
SRCDIR := src
RESDIR := res
CFLAGS := -I$(SRCDIR) -I$(RESDIR) -Wall -Wextra -O2 -std=c11 #-Werror
LDSCRIPT := md.ld
SOURCES_C := $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*/*.c)
SOURCES_ASM := $(wildcard $(SRCDIR)/*.s) $(wildcard $(SRCDIR)/*/*.s)
RESOURCES_LIST := $(wildcard $(RESDIR)/*.bin) $(wildcard $(RESDIR)/*/*.bin)
OUTPUT_FILE := prg
LIBS :=

# Environment - GBIN should point to the m68k-elf-* binaries
GBIN := /opt/gendev/bin
CC_HOST := cc
CC := $(GBIN)/m68k-elf-gcc
AS := $(GBIN)/m68k-elf-gcc
LD := $(GBIN)/m68k-elf-ld
NM := $(GBIN)/m68k-elf-nm
OBJCOPY := $(GBIN)/m68k-elf-objcopy
BINCLUDE := util/binclude
MEGALOADER := util/megaloader

# Compiler, assembler, and linker flag setup
CFLAGS+= -Wno-strict-aliasing -ffreestanding 
CFLAGS+= -fomit-frame-pointer -fno-defer-pop -frename-registers -fshort-enums
CFLAGS+=-mcpu=$(CPUTYPE)
# CFLAGS+= -ffunction-sections -fdata-sections -fconserve-stack
ASFLAGS:=$(CFLAGS)
LDFLAGS:=--gc-sections -nostdlib
LDFLAGS+= -T$(LDSCRIPT)

OBJECTS_C := $(addsuffix .o,$(addprefix $(OUTPUT_DIR),$(basename $(SOURCES_C))))
OBJECTS_ASM := $(addsuffix .o,$(addprefix $(OUTPUT_DIR),$(basename $(SOURCES_ASM))))
OBJECTS_RES := $(addsuffix .o,$(addprefix $(OUTPUT_DIR),$(basename $(RESOURCES_LIST))))

.PHONY: all

all: binclude_bin megaloader_bin $(OUTPUT_FILE).elf $(OUTPUT_FILE).bin

megaloader_bin:
	$(CC_HOST) -D_DEFAULT_SOURCE util/megaloader.c -o $(MEGALOADER) -O3 -std=c11

binclude_bin:
	$(CC_HOST) util/binclude.c -o $(BINCLUDE) -O3 -std=c11

$(OUTPUT_FILE).bin: $(OUTPUT_FILE).elf
	$(OBJCOPY) -O binary $< temp.bin
	dd if=temp.bin of=$@ bs=2K conv=sync
	rm $(OUTPUT_FILE).elf
	rm temp.bin

# TODO: Why are all of OBJECTS_RES not being built first?
$(OUTPUT_FILE).elf: $(OBJECTS_RES) $(OBJECTS_C) $(OBJECTS_ASM)
	$(LD) -o $@ $(LDFLAGS) $(OBJECTS_RES) $(OBJECTS_C) $(OBJECTS_ASM) $(LIBS)

%.o: %.bin
	$(BINCLUDE) $< $(subst /,_,$(basename $<))
	mv $(subst /,_,$(basename $<)).h res/
	mv $(subst /,_,$(basename $<)).c res/
	$(CC) $(CFLAGS) -c res/$(subst /,_,$(basename $<)).c -o $@
	rm res/$(subst /,_,$(basename $<)).c

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) -c $< -o $@

gens: all
	@exec gens $(OUTPUT_FILE).bin 2> /dev/null

flash: all
	@exec $(MEGALOADER) md $(OUTPUT_FILE).bin /dev/ttyUSB0 2> /dev/null

clean:
	@-rm -f $(OBJECTS_C) $(OBJECTS_ASM) $(OUTPUT_FILE).bin $(BINCLUDE)
#	@-rm -f $(RESDIR)/$(shell find . -name '*.o')  $(RESDIR)/res_*.h
	@-rm -f $(OBJECTS_RES) $(RESDIR)/res_*.h
