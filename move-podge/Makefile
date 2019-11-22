# Configuration
OUTPUT_FILE := move-podge
OUTPUT_EXT := gen
OUTPUT_VERSION := wip

CPUTYPE := 68000
SRCDIR := src
COMMONSRCDIR := ../common/src
RESDIR := res
OBJDIR := obj
UTILDIR := ../util
CFLAGS := -I$(SRCDIR) -I$(OBJDIR) -I$(COMMONSRCDIR) -Wall -Wextra -O2 -std=c11 -Wno-unused-function #-Werror
HOSTCFLAGS := -Os -std=c11
LDSCRIPT := ../common/md.ld
SOURCES_C := $(shell find $(SRCDIR)/ -type f -name '*.c')
SOURCES_ASM := $(shell find $(SRCDIR)/ -type f -name '*.s')
COMMONSOURCES_C := $(shell find $(COMMONSRCDIR)/ -type f -name '*.c')
COMMONSOURCES_ASM := $(shell find $(COMMONSRCDIR)/ -type f -name '*.s')
RESOURCES_LIST := $(shell find $(RESDIR)/ -type f -name '*.bin')
LIBS :=

# Experimental PNG rules
IMAGES_PNG := $(shell find $(RESDIR)/ -type f -name '*.png')
RESOURCES_LIST += $(patsubst $(RESDIR)/%.png,$(OBJDIR)/%_gfx.bin,$(IMAGES_PNG))
RESOURCES_LIST += $(patsubst $(RESDIR)/%.png,$(OBJDIR)/%_pal.bin,$(IMAGES_PNG))

$(OBJDIR)/%_gfx.bin: $(RESDIR)/%.png
	$(PNGTO) -p "3210" $< $@
$(OBJDIR)/%H16_gfx.bin: $(RESDIR)/%H16.png
	$(PNGTO) -p "3210" -H 16 $< $@
$(OBJDIR)/%H24_gfx.bin: $(RESDIR)/%H24.png
	$(PNGTO) -p "3210" -H 24 $< $@
$(OBJDIR)/%H32_gfx.bin: $(RESDIR)/%H32.png
	$(PNGTO) -p "3210" -H 32 $< $@
$(OBJDIR)/%_pal.bin: $(RESDIR)/%.png
	$(PNGTO) -c 0000BBB0GGG0RRR0 --num-colors=16 $< $@

include ../common/md-rules.mk
