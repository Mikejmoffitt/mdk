# md-framework common build rules.

# Determine the target platform.
# Valid platforms:
#   md
#   c2
ifeq ($(PLATFORM),)
PLATFORM := md
endif

# Environment
MD_ENV := /opt/toolchains/gen/m68k-elf
GBIN := $(MD_ENV)/bin

MDKSRCDIR := $(MDKROOT)/src
LDSCRIPT := $(MDKROOT)/$(PLATFORM).ld
UTILDIR := $(MDKROOT)/util
MDKSOURCES_C := $(shell find $(MDKSRCDIR)/ -type f -name '*.c')
MDKSOURCES_ASM := $(shell find $(MDKSRCDIR)/ -type f -name '*.s')

HOSTCFLAGS := -O3 -std=c11
CC_HOST := cc
CC := $(GBIN)/m68k-elf-gcc
AS := $(GBIN)/m68k-elf-gcc
LD := $(GBIN)/m68k-elf-ld
NM := $(GBIN)/m68k-elf-nm
OBJCOPY := $(GBIN)/m68k-elf-objcopy
BIN2S := $(UTILDIR)/bin2s
BIN2H := $(UTILDIR)/bin2h
BINPAD := $(UTILDIR)/binpad
BSPLIT := $(UTILDIR)/bsplit
SPLIT := split
MEGALOADER := $(UTILDIR)/megaloader
PNGTO := $(UTILDIR)/pngto
BLASTEM := $(UTILDIR)/blastem64-*/blastem

# Target-specific.
ifeq ($(TARGET_SYSTEM),)
TARGET_SYSTEM = MDK_TARGET_MD
endif

ifeq ($(TARGET_SYSTEM),MDK_TARGET_C2)
MDK_C2_TESTROM := zunkyou
ifeq ($(MDK_C2_ADPCM),)
MDK_C2_ADPCM = /dev/zero
endif
endif

# If the user didn't specify the MDK root dir, assume it's at the project root.
ifeq ($(MDKROOT),)
MDKROOT := mdk
endif

# Compiler, assembler, and linker flag setup
CFLAGS += -Wno-strict-aliasing -ffreestanding
CFLAGS += -fomit-frame-pointer -fno-defer-pop -frename-registers -fshort-enums
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -mcpu=68000
CFLAGS += -I$(SRCDIR) -I$(MDKSRCDIR) -I.
CFLAGS += -O3
CFLAGS += -std=c11
CFLAGS += -Wall -Wextra -Wno-unused-function
CFLAGS += # -fno-store-merging # Needed to avoid breakage with GCC8.
CFLAGS += -ffunction-sections -fdata-sections -fconserve-stack
CFLAGS += -D$(TARGET_SYSTEM)
ASFLAGS := -Wa,-I$(SRCDIR) -Wa,-I$(OBJDIR) -Wa,-I$(MDKSRCDIR)
# LDFLAGS := -L/usr/lib/gcc-cross/m68k-linux-gnu/8
LDFLAGS := -L$(MD_ENV)/m68k-elf/lib -L$(MD_ENV)/lib/gcc/m68k-elf/6.3.0
LDFLAGS += --gc-sections -nostdlib
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Map $(PROJECT_NAME).map
LIBS += -lgcc


# Naming intermediates
OUTPUT_ELF := $(OBJDIR)/$(PROJECT_NAME).elf
OUTPUT_GEN := $(PROJECT_NAME).gen
OBJECTS_C := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES_C)) \
  $(patsubst $(MDKSRCDIR)/%.c,$(OBJDIR)/%.o,$(MDKSOURCES_C))
OBJECTS_ASM := $(patsubst $(SRCDIR)/%.s,$(OBJDIR)/%.o,$(SOURCES_ASM)) \
  $(patsubst $(MDKSRCDIR)/%.s,$(OBJDIR)/%.o,$(MDKSOURCES_ASM))
OBJECTS_RES := $(OBJDIR)/res.o

RES_HEADER := res.h

.PHONY: all vars $(RES_HEADER) $(MDK_C2_TESTROM) test_c2

# Generic var for additional files, etc. that are a build prereq.
EXTERNAL_DEPS ?=
EXTERNAL_ARTIFACTS ?=

ifeq ($(TARGET_SYSTEM),MDK_TARGET_C2)
all: $(BINCLUDE) $(MDK_C2_TESTROM)
else
all: $(BLASTEM) $(BINCLUDE) $(MEGALOADER) $(OUTPUT_GEN)
endif

# Generic target that is intended to be overridden.
ext_deps: $(EXTERNAL_DEPS)

vars:
	@echo "CFLAGS is" "$(CFLAGS)"
	@echo "MDKSOURCES_C is" "$(MDKSOURCES_C)"
	@echo "MDKSOURCES_ASM is" "$(MDKSOURCES_ASM)"
	@echo "SOURCES_C is" "$(SOURCES_C)"
	@echo "SOURCES_ASM is" "$(SOURCES_ASM)"
	@echo "OBJECTS_C is" "$(OBJECTS_C)"
	@echo "OBJECTS_ASM is" "$(OBJECTS_ASM)"

# An archive for Blastem is included; this just unpacks it.
$(BLASTEM):
	cd $(UTILDIR) && tar -xf blastem64-*.tar.gz

$(MEGALOADER): $(UTILDIR)/megaloader.c
	@$(CC_HOST) -D_DEFAULT_SOURCE $< -o $@ $(HOSTCFLAGS)

$(BIN2S): $(UTILDIR)/bin2s.c
	@$(CC_HOST) $^ -o $@ $(HOSTCFLAGS)

$(BIN2H): $(UTILDIR)/bin2h.c
	@$(CC_HOST) $^ -o $@ $(HOSTCFLAGS)

$(BINPAD): $(UTILDIR)/binpad.c
	@$(CC_HOST) $^ -o $@ $(HOSTCFLAGS)

$(BSPLIT): $(UTILDIR)/bsplit.c
	@$(CC_HOST) $^ -o $@ $(HOSTCFLAGS)

$(PNGTO): $(UTILDIR)/pngto.c $(UTILDIR)/musl_getopt.c $(UTILDIR)/lodepng.c $(UTILDIR)/indexedimage.c
	@$(CC_HOST) $^ -o $@ -DLODEPNG_NO_COMPILE_ENCODER $(HOSTCFLAGS)

$(OUTPUT_GEN): $(OUTPUT_ELF) $(BINPAD)
	@bash -c 'printf " \e[36m[ PAD ]\e[0m ... --> $@\n"'
	$(OBJCOPY) -O binary $< $@
ifeq ($(TARGET_SYSTEM),MDK_TARGET_C2)
	$(BINPAD) $@ 0x200000
else
	$(BINPAD) $@
endif
	@bash -c 'printf "\e[92m [ OK! ]\e[0m --> $(OUTPUT_GEN)\n"'

$(OBJDIR)/$(PROJECT_NAME).elf: $(OBJECTS_RES) $(OBJECTS_C) $(OBJECTS_ASM)
	@mkdir -p $(dir $@)
	@bash -c 'printf " \e[36m[ LNK ]\e[0m ... --> $@\n"'
	$(LD) -o $@ $(LDFLAGS) $(OBJECTS_RES) $(OBJECTS_C) $(OBJECTS_ASM) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJECTS_RES) $(RES_HEADER) ext_deps
	@mkdir -p $(dir $@)
	@bash -c 'printf " \e[96m[  C  ]\e[0m $< --> $@\n"'
	$(CC) $(CFLAGS) -c $< -o $@
ifneq ($(MDK_WANT_ASM_OUT),)
	$(CC) $(CFLAGS) -S $< -o $@.asm
endif

$(OBJDIR)/%.o: $(SRCDIR)/%.s $(OBJECTS_RES) ext_deps
	@mkdir -p $(dir $@)
	@bash -c 'printf " \e[33m[ ASM ]\e[0m $< --> $@\n"'
	$(AS) $(ASFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(MDKSRCDIR)/%.c $(OBJECTS_RES) ext_deps
	@mkdir -p $(dir $@)
	@bash -c 'printf " \e[96m[ C:C ]\e[0m $< --> $@\n"'
	$(CC) $(CFLAGS) -c $< -o $@
ifneq ($(MDK_WANT_ASM_OUT),)
	$(CC) $(CFLAGS) -S $< -o $@.asm
endif

$(OBJDIR)/%.o: $(MDKSRCDIR)/%.s $(OBJECTS_RES) ext_deps
	@mkdir -p $(dir $@	)
	@bash -c 'printf " \e[33m[C:ASM]\e[0m $< --> $@\n"'
	$(AS) $(ASFLAGS) -c $< -o $@

# Converts res.s and other intermediate files generated by util
$(OBJDIR)/%.o: $(OBJDIR)/%.s
	@bash -c 'printf " \e[33m[B:ASM]\e[0m $< --> $@\n"'
	$(AS) -c $< -o $@

# Converts a file to object files
$(OBJDIR)/res.s: $(BIN2S) $(RESOURCES_LIST)
	mkdir -p $(dir $@)
	@bash -c 'printf " \e[95m[ BIN ]\e[0m $^ --> $@\n"'
	$^ > $@

# Generates header entries for resource data
$(RES_HEADER): $(BIN2H) $(RESOURCES_LIST)
	@bash -c 'printf " \e[95m[RES.H]\e[0m $^ --> $@\n"'
	@printf '#ifndef _RES_AUTOGEN_H\n#define _RES_AUTOGEN_H\n' > $@
	$^ >> $@
	@printf '#endif  // _RES_AUTOGEN_H\n' >> $@

res_post:
	printf

flash: $(OUTPUT_GEN) $(MEGALOADER)
	$(MEGALOADER) md $< /dev/ttyUSB0 2> /dev/null

ifeq ($(TARGET_SYSTEM),MDK_TARGET_C2)

debug: $(MDK_C2_TESTROM)
	exec mame $< -rompath $(shell pwd) -debug -r 640x480

test: $(MDK_C2_TESTROM)
	exec mame $< -rompath $(shell pwd)

$(MDK_C2_TESTROM): $(OUTPUT_GEN) $(BSPLIT)
	# MAME does not have a generic System C2 target, so Zunzunkyou no Yabou is used.
	mkdir -p $@
	$(BSPLIT) s $< prg_even.bin prg_odd.bin
	$(SPLIT) -b 524288 prg_even.bin
	mv xaa $@/epr-16812.ic32
	mv xab $@/epr-16814.ic34
	$(SPLIT) -b 524288 prg_odd.bin
	mv xaa $@/epr-16811.ic31
	mv xab $@/epr-16813.ic33
	dd if=$(MDK_C2_ADPCM) of=$@/epr-16810.ic4 bs=1 count=524288
	rm prg_even.bin prg_odd.bin

else

debug: $(OUTPUT_GEN) $(BLASTEM)
	$(BLASTEM) -m gen -d $<

test: $(OUTPUT_GEN) $(BLASTEM)
	bash -c 'PULSE_LATENCY_MSEC=80 $(BLASTEM) -m gen $<'

mame: $(OUTPUT_GEN)
	exec mame megadrij -cart $< -debug -r 640x480

endif

clean:
	-rm -f $(OBJECTS_C) $(OBJECTS_ASM) $(OUTPUT_GEN)
	-rm -f $(OUTPUT_ELF)
	-rm -f $(OBJECTS_RES) $(OBJDIR)/res.s $(RES_HEADER)
	-rm -rf $(OBJDIR)
	-rm -f $(PROJECT_NAME).map
	-rm -rf zunkyou
	echo $(EXTERNAL_ARTIFACTS) | xargs --no-run-if-empty rm -f $(EXTERNAL_ARTIFACTS)
