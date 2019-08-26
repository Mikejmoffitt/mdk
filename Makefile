# Configuration
CPUTYPE := 68000
SRCDIR := src
RESDIR := res
CFLAGS := -I$(SRCDIR) -I$(RESDIR) -std=c11 -O2 -fno-strict-aliasing -fno-math-errno -fno-strict-overflow -Wall -Wextra -Wno-unused-function #-Werror
LDSCRIPT := md.ld
SOURCES_C := $(shell find $(SRCDIR)/ -type f -name '*.c')
SOURCES_ASM := $(shell find $(SRCDIR)/ -type f -name '*.s')
RESOURCES_LIST := $(shell find $(RESDIR)/ -type f -name '*.bin')
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
BLASTEM := util/blastem64-0.5.1/blastem

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

all: $(BLASTEM) $(BINCLUDE) $(MEGALOADER) $(OUTPUT_FILE).elf $(OUTPUT_FILE).bin

# An archive for Blastem is included; this just unpacks it.
$(BLASTEM):
	cd util && tar -xf blastem64-0.5.1.tar.gz

$(MEGALOADER):
	@$(CC_HOST) -D_DEFAULT_SOURCE util/megaloader.c -o $(MEGALOADER) -O3 -std=c11

$(BINCLUDE): util/binclude.c
	@$(CC_HOST) $^ -o $(BINCLUDE) -O3 -std=c11

$(OUTPUT_FILE).bin: $(OUTPUT_FILE).elf
	@$(OBJCOPY) -O binary $< temp.bin
	@dd if=temp.bin of=$@ bs=512 conv=sync status=none
	@rm $(OUTPUT_FILE).elf
	@rm temp.bin
	@bash -c 'printf "\e[92m [ OK! ]\e[0m --> $(OUTPUT_FILE).bin\n"'

$(OUTPUT_FILE).elf: $(OBJECTS_RES) $(OBJECTS_C) $(OBJECTS_ASM)
	@bash -c 'printf " \e[36m[ LNK ]\e[0m ... --> $@\n"'
	@$(LD) -o $@ $(LDFLAGS) $(OBJECTS_RES) $(OBJECTS_C) $(OBJECTS_ASM) $(LIBS)

%.o: %.bin
	@bash -c 'printf " \e[95m[ BIN ]\e[0m $< --> $@\n"'
	@$(BINCLUDE) $< $(subst /,_,$(basename $<)) > /dev/null
	@mv $(subst /,_,$(basename $<)).h res/
	@mv $(subst /,_,$(basename $<)).c res/
	@$(CC) $(CFLAGS) -c res/$(subst /,_,$(basename $<)).c -o $@
	@rm res/$(subst /,_,$(basename $<)).c

%.o: %.c $(OBJECTS_RES)
	@bash -c 'printf " \e[96m[  C  ]\e[0m $< --> $@\n"'
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s $(OBJECTS_RES)
	@bash -c 'printf " \e[33m[ ASM ]\e[0m $< --> $@\n"'
	@$(AS) $(ASFLAGS) -c $< -o $@

flash: all
	@exec $(MEGALOADER) md $(OUTPUT_FILE).bin /dev/ttyUSB0 2> /dev/null

debug: all
	@exec $(BLASTEM) -m gen -d $(OUTPUT_FILE).bin
	
test: all
	@exec $(BLASTEM) -m gen $(OUTPUT_FILE).bin
	
clean:
	@-rm -f $(OBJECTS_C) $(OBJECTS_ASM) $(OUTPUT_FILE).bin
	@-rm -f $(OBJECTS_RES) $(RESDIR)/res_*.h
