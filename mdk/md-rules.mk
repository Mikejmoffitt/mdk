# md-framework common build rules.

# Environment
MDK_ENV := /opt/toolchains/m68k-elf
MDK_BIN := $(MDK_ENV)/bin

WSLENV ?= not_wsl
ifndef WSLENV
HOST_WSL := 1
endif


# If the user didn't specify the MDK root dir, assume it's at the project root.
ifeq ($(MDKROOT),)
MDKROOT := mdk
endif

MDKSRCDIR := $(MDKROOT)/src
LDSCRIPT := $(MDKROOT)/md.ld
UTILDIR := $(MDKROOT)/util
MDKSOURCES_C := $(shell find $(MDKSRCDIR)/ -type f -name '*.c')
MDKSOURCES_ASM := $(shell find $(MDKSRCDIR)/ -type f -name '*.s')

# Target-specific
ifeq ($(TARGET_SYSTEM),)
TARGET_SYSTEM = MDK_TARGET_MD
endif

ifeq ($(TARGET_SYSTEM),MDK_TARGET_C2)
MDK_C2_TESTROM := zunkyou
ifeq ($(MDK_C2_ADPCM),)
MDK_C2_ADPCM = /dev/zero
endif
endif

CC_HOST := cc
CC := $(MDK_BIN)/m68k-elf-gcc
CPPC := $(MDK_BIN)/m68k-elf-g++
AS := $(MDK_BIN)/m68k-elf-as
LD := $(MDK_BIN)/m68k-elf-gcc
NM := $(MDK_BIN)/m68k-elf-nm
OBJCOPY := $(MDK_BIN)/m68k-elf-objcopy

SPLIT := split
# Utilities packed in
BIN2S := $(UTILDIR)/core/bin2s
BIN2H := $(UTILDIR)/core/bin2h
BINPAD := $(UTILDIR)/core/binpad
BSPLIT := $(UTILDIR)/core/bsplit
PNGTO := $(UTILDIR)/image/pngto/pngto
PNG2CSP := $(UTILDIR)/png2csp/png2csp
# Sik's tools
MDTOOLS := $(UTILDIR)/mdtools
MDTILER := $(UTILDIR)/mdtools/mdtiler/tool/mdtiler
# TODO: Various echo tools

# Emulator(s)
ifdef HOST_WSL
BLASTEM := $(UTILDIR)/emu/blastem/blastem-win64-*/blastem.exe
else
BLASTEM := $(UTILDIR)/emu/blastem/blastem64-*/blastem
endif
MEGALOADER := $(UTILDIR)/debug/megaloader/megaloader

# FLags passed in.
FLAGS ?=

# Flags shared by both C and C++.
COMMON_FLAGS := $(FLAGS)
COMMON_FLAGS += -mcpu=68000
COMMON_FLAGS += $(OPTLEVEL)
COMMON_FLAGS += -fomit-frame-pointer
COMMON_FLAGS += -frename-registers -fshort-enums
COMMON_FLAGS += -Wall -Wextra -Wno-unused-function
COMMON_FLAGS += -ffreestanding
COMMON_FLAGS += -ffunction-sections -fdata-sections -fconserve-stack
COMMON_FLAGS += -fwrapv
COMMON_FLAGS += -fno-gcse
COMMON_FLAGS += -I$(SRCDIR) -I$(MDKSRCDIR) -I.
COMMON_FLAGS += -D$(TARGET_SYSTEM)

# For C.
CFLAGS := $(COMMON_FLAGS)
CFLAGS += -std=gnu2x
CFLAGS += -Wno-strict-aliasing

# For C++.
CPPFLAGS := $(COMMON_FLAGS)
CPPFLAGS += -std=gnu++2b

# For ASM.
ASFLAGS := -m68000 --bitwise-or -I$(SRCDIR) -I$(OBJDIR) -I$(MDKSRCDIR)
ASFLAGS += --register-prefix-optional

# Linker.
GCC_VER := $(shell $(CC) -dumpversion)
LDFLAGS += -nostartfiles
LDFLAGS += -T $(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map $(PROJECT_NAME).map

LIBS := -L $(MDK_ENV)/m68k-elf/lib -lnosys
LIBS += -L $(MDK_ENV)/lib/gcc/m68k-elf/$(GCC_VER) -lgcc

# C (on the host, for tools, etc)
HOSTCFLAGS := -O3 -std=gnu11

# Naming intermediates
OUTPUT_ELF := $(OBJDIR)/$(PROJECT_NAME).elf
OUTPUT_GEN := $(PROJECT_NAME).gen
OBJECTS_C := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES_C)) \
  $(patsubst $(MDKSRCDIR)/%.c,$(OBJDIR)/%.o,$(MDKSOURCES_C))
OBJECTS_CPP := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES_CPP)) \
  $(patsubst $(MDKSRCDIR)/%.cpp,$(OBJDIR)/%.o,$(MDKSOURCES_CPP))
OBJECTS_ASM := $(patsubst $(SRCDIR)/%.s,$(OBJDIR)/%.o,$(SOURCES_ASM)) \
  $(patsubst $(MDKSRCDIR)/%.s,$(OBJDIR)/%.o,$(MDKSOURCES_ASM))
OBJECTS_RES := $(OBJDIR)/res.o

RES_HEADER := res.h

.PHONY: all vars $(RES_HEADER) $(MDK_C2_TESTROM) test_c2 $(EXTERNAL_DEPS)

# Generic var for additional files, etc. that are a build prereq.
EXTERNAL_DEPS ?=
EXTERNAL_ARTIFACTS ?=

ifeq ($(TARGET_SYSTEM),MDK_TARGET_C2)
all: $(BINCLUDE) $(MDK_C2_TESTROM)
else
all: $(BLASTEM) $(BINCLUDE) $(MEGALOADER) $(OUTPUT_GEN)
endif

vars:
	@echo "GCC_VER is" "$(GCC_VER)"
	@echo "CFLAGS is" "$(CFLAGS)"
	@echo "CPPFLAGS is" "$(CPPFLAGS)"
	@echo "LDFLAGS is" "$(LDFLAGS)"
	@echo "MDKSOURCES_C is" "$(MDKSOURCES_C)"
	@echo "MDKSOURCES_ASM is" "$(MDKSOURCES_ASM)"
	@echo "SOURCES_C is" "$(SOURCES_C)"
	@echo "SOURCES_CPP is" "$(SOURCES_CPP)"
	@echo "SOURCES_ASM is" "$(SOURCES_ASM)"
	@echo "OBJECTS_C is" "$(OBJECTS_C)"
	@echo "OBJECTS_CPP is" "$(OBJECTS_CPP)"
	@echo "OBJECTS_ASM is" "$(OBJECTS_ASM)"

# An archive for Blastem is included; this just unpacks it.
$(BLASTEM):
ifdef HOST_WSL
	cd $(UTILDIR)/emu/blastem/ && unzip blastem-win64-*.zip
else
	cd $(UTILDIR)/emu/blastem/ && tar -xf blastem64-*.tar.gz
endif

$(MEGALOADER): $(UTILDIR)/debug/megaloader/megaloader.c
	@$(CC_HOST) -D_DEFAULT_SOURCE $< -o $@ $(HOSTCFLAGS)

$(MDTOOLS):
	git clone git@github.com:sikthehedgehog/mdtools $@

$(MDTILER): $(MDTOOLS)
	cd $(MDTOOLS)/mdtiler/tool && CC=$(CC_HOST) make

$(BIN2S): $(UTILDIR)/core/bin2s.c
	@$(CC_HOST) $^ -o $@ $(HOSTCFLAGS)

$(BIN2H): $(UTILDIR)/core/bin2h.c
	@$(CC_HOST) $^ -o $@ $(HOSTCFLAGS)

$(BINPAD): $(UTILDIR)/core/binpad.c
	@$(CC_HOST) $^ -o $@ $(HOSTCFLAGS)

$(BSPLIT): $(UTILDIR)/core/bsplit.c
	@$(CC_HOST) $^ -o $@ $(HOSTCFLAGS)

$(PNG2CSP):
	$(MAKE) -C $(UTILDIR)/png2csp/

$(PNGTO): $(UTILDIR)/image/pngto/pngto.c $(UTILDIR)/image/pngto/musl_getopt.c $(UTILDIR)/image/pngto/lodepng.c $(UTILDIR)/image/pngto/indexedimage.c
	@$(CC_HOST) $^ -I $(UTILDIR)/image/pngto -o $@ -DLODEPNG_NO_COMPILE_ENCODER $(HOSTCFLAGS)

$(OUTPUT_GEN): $(OUTPUT_ELF) $(BINPAD)
	@bash -c 'printf " \e[36m[ PAD ]\e[0m ... --> $@\n"'
	$(OBJCOPY) -O binary $< $@
ifeq ($(TARGET_SYSTEM),MDK_TARGET_C2)
	$(BINPAD) $@ 0x200000
else
	$(BINPAD) $@
endif
	@bash -c 'printf "\e[92m [ OK! ]\e[0m --> $(OUTPUT_GEN)\n"'

$(OBJDIR)/$(PROJECT_NAME).elf: $(OBJECTS_RES) $(OBJECTS_C) $(OBJECTS_CPP) $(OBJECTS_ASM)
	@mkdir -p $(dir $@)
	@bash -c 'printf " \e[36m[ LNK ]\e[0m ... --> $@\n"'
	$(LD) -o $@ $(LDFLAGS) $(OBJECTS_RES) $(OBJECTS_C) $(OBJECTS_CPP) $(OBJECTS_ASM) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(OBJECTS_RES) $(RES_HEADER) $(EXTERNAL_DEPS)
	@mkdir -p $(dir $@)
	@bash -c 'printf " \e[96m[  C  ]\e[0m $< --> $@\n"'
	$(CC) $(CFLAGS) -c $< -o $@
ifneq ($(MDK_WANT_ASM_OUT),)
	$(CC) $(CFLAGS) -S $< -o $@.asm
endif

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(OBJECTS_RES) $(RES_HEADER) $(EXTERNAL_DEPS)
	@mkdir -p $(dir $@)
	@bash -c 'printf " \e[96m[ CPP ]\e[0m $< --> $@\n"'
	$(CPPC) $(CPPFLAGS) -c $< -o $@
ifneq ($(MDK_WANT_ASM_OUT),)
	$(CPPC) $(CPPFLAGS) -S $< -o $@.asm
endif

$(OBJDIR)/%.o: $(SRCDIR)/%.s $(OBJECTS_RES) $(EXTERNAL_DEPS)
	@mkdir -p $(dir $@)
	@bash -c 'printf " \e[33m[ ASM ]\e[0m $< --> $@\n"'
	$(AS) $(ASFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(MDKSRCDIR)/%.c $(OBJECTS_RES) $(EXTERNAL_DEPS)
	@mkdir -p $(dir $@)
	@bash -c 'printf " \e[96m[ C:C ]\e[0m $< --> $@\n"'
	$(CC) $(CFLAGS) -c $< -o $@
ifneq ($(MDK_WANT_ASM_OUT),)
	$(CC) $(CFLAGS) -S $< -o $@.asm
endif

$(OBJDIR)/%.o: $(MDKSRCDIR)/%.s $(OBJECTS_RES) $(EXTERNAL_DEPS)
	@mkdir -p $(dir $@	)
	@bash -c 'printf " \e[33m[C:ASM]\e[0m $< --> $@\n"'
	$(AS) $(ASFLAGS) -c $< -o $@

# Converts res.s and other intermediate files generated by util
$(OBJDIR)/%.o: $(OBJDIR)/%.s
	@bash -c 'printf " \e[33m[B:ASM]\e[0m $< --> $@\n"'
	$(AS) -c $< -o $@

# Converts a file to object files
$(OBJDIR)/res.s: $(BIN2S) $(EXTERNAL_DEPS)
	mkdir -p $(dir $@)
	@bash -c 'printf " \e[95m[RES.S]\e[0m $(RESDIR) --> $@\n"'
	$< $(shell find $(RESDIR) -type f -name '*') > $@

# Generates header entries for resource data
$(RES_HEADER): $(BIN2H) $(EXTERNAL_DEPS)
	@bash -c 'printf " \e[95m[RES.H]\e[0m $(RESDIR) --> $@\n"'
	@printf '#ifndef _RES_AUTOGEN_H\n#define _RES_AUTOGEN_H\n' > $@
	$< $(shell find $(RESDIR) -type f -name '*') >> $@
	@printf '#endif  // _RES_AUTOGEN_H\n' >> $@

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
	$(BLASTEM) -m gen $<

mame: $(OUTPUT_GEN)
	exec mame megadrij -cart $< -debug -r 640x480

endif

clean:
	-rm -f $(OBJECTS_C) $(OBJECTS_CPP) $(OBJECTS_ASM) $(OUTPUT_GEN)
	-rm -f $(OUTPUT_ELF)
	-rm -f $(OBJECTS_RES) $(OBJDIR)/res.s $(RES_HEADER)
	-rm -rf $(OBJDIR)
	-rm -f $(PROJECT_NAME).map
	-rm -rf zunkyou
	echo $(EXTERNAL_ARTIFACTS) | xargs --no-run-if-empty rm -rf $(EXTERNAL_ARTIFACTS)


toolchain:
