# Configuration
CPUTYPE := 68000
SRCDIR := src
RESDIR := res
OBJDIR := obj
CFLAGS := -I$(SRCDIR) -I$(OBJDIR) -Wall -Wextra -O2 -std=c11 -Wno-unused-function #-Werror
HOSTCFLAGS := -Os -std=c11
LDSCRIPT := md.ld
SOURCES_C := $(shell find $(SRCDIR)/ -type f -name '*.c')
SOURCES_ASM := $(shell find $(SRCDIR)/ -type f -name '*.s')
RESOURCES_LIST := $(shell find $(RESDIR)/ -type f -name '*.bin')
OUTPUT_FILE := md-framework
OUTPUT_EXT := gen
OUTPUT_VERSION := wip
LIBS :=

# Environment - GBIN should point to the m68k-elf-* binaries
GBIN := /opt/gendev/bin
CC_HOST := cc
CC := $(GBIN)/m68k-elf-gcc
AS := $(GBIN)/m68k-elf-gcc
LD := $(GBIN)/m68k-elf-ld
NM := $(GBIN)/m68k-elf-nm
OBJCOPY := $(GBIN)/m68k-elf-objcopy
BIN2S := util/bin2s
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

# Naming intermediates
OUTPUT_ELF := $(OBJDIR)/$(OUTPUT_FILE).elf
OUTPUT_UNPAD := $(OBJDIR)/$(OUTPUT_FILE).gen
OUTPUT_GEN := $(OUTPUT_FILE).gen
OBJECTS_C := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES_C))
OBJECTS_ASM := $(patsubst $(SRCDIR)/%.s,$(OBJDIR)/%.o,$(SOURCES_ASM))
OBJECTS_RES := $(OBJDIR)/res.o

.PHONY: all objdir

all: $(BLASTEM) $(BINCLUDE) $(MEGALOADER) $(OUTPUT_GEN)

# An archive for Blastem is included; this just unpacks it.
$(BLASTEM):
	cd util && tar -xf blastem64-0.5.1.tar.gz

$(MEGALOADER):
	@$(CC_HOST) -D_DEFAULT_SOURCE util/megaloader.c -o $@ $(HOSTCFLAGS)

$(BIN2S): util/bin2s.c
	@$(CC_HOST) $^ -o $@ -Os  $(HOSTCFLAGS)

$(OUTPUT_GEN): $(OUTPUT_ELF)
	@bash -c 'printf " \e[36m[ PAD ]\e[0m ... --> $@\n"'
	@$(OBJCOPY) -O binary $< $(OUTPUT_UNPAD)
	@dd if=$(OUTPUT_UNPAD) of=$@ bs=512 conv=sync status=none
	@rm $(OUTPUT_UNPAD)
	@bash -c 'printf "\e[92m [ OK! ]\e[0m --> $(OUTPUT_GEN)\n"'

$(OBJDIR)/$(OUTPUT_FILE).elf: $(OBJECTS_RES) $(OBJECTS_C) $(OBJECTS_ASM)
	@mkdir -p $(notdir $@)
	@bash -c 'printf " \e[36m[ LNK ]\e[0m ... --> $@\n"'
	@$(LD) -o $@ $(LDFLAGS) $(OBJECTS_RES) $(OBJECTS_C) $(OBJECTS_ASM) $(LIBS)

# Converts a file to .c and .h files
$(OBJDIR)/res.s: $(BIN2S) $(RESOURCES_LIST)
	@mkdir -p $(dir $@)
	@bash -c 'printf " \e[95m[ BIN ]\e[0m $^ --> $@\n"'
	@$^ > $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJECTS_RES) 
	@mkdir -p $(dir $@)
	@bash -c 'printf " \e[96m[  C  ]\e[0m $< --> $@\n"'
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.s $(OBJECTS_RES)
	@mkdir -p $(dir $@)
	@bash -c 'printf " \e[33m[ ASM ]\e[0m $< --> $@\n"'
	@$(AS) $(ASFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(OBJDIR)/%.s
	@bash -c 'printf " \e[33m[B:ASM]\e[0m $< --> $@\n"'
	@$(AS) $(ASFLAGS) -c $< -o $@

flash: all
	@exec $(MEGALOADER) md $(OUTPUT_GEN) /dev/ttyUSB0 2> /dev/null

debug: all
	@exec $(BLASTEM) -m gen -d $(OUTPUT_GEN)

test: all
	@exec $(BLASTEM) -m gen $(OUTPUT_GEN)

clean:
	@-rm -f $(OBJECTS_C) $(OBJECTS_ASM) $(OUTPUT_GEN)
	@-rm -f $(OUTPUT_ELF) $(OUTPUT_UNPAD)
	@-rm -f $(OBJECTS_RES) $(OBJDIR)/res.s
