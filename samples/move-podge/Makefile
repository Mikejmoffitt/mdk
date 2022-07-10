# Configuration
PROJECT_NAME := move_podge

MDKROOT := ../../mdk

SRCDIR := src
RESDIR := res
OBJDIR := obj
SOURCES_C := $(shell find $(SRCDIR)/ -type f -name '*.c')
SOURCES_ASM := $(shell find $(SRCDIR)/ -type f -name '*.s')
RESOURCES_LIST := $(shell find $(RESDIR)/ -type f -name '*.bin')
LIBS :=

# Experimental PNG rules
IMAGES_PNG := $(shell find $(RESDIR)/ -type f -name '*.png')
RESOURCES_LIST += $(patsubst $(RESDIR)/%.png,$(OBJDIR)/%_gfx.bin,$(IMAGES_PNG))
RESOURCES_LIST += $(patsubst $(RESDIR)/%.png,$(OBJDIR)/%_pal.bin,$(IMAGES_PNG))

include $(MDKROOT)/md-rules.mk

$(OBJDIR)/%_gfx.bin: $(PNGTO) $(RESDIR)/%.png
	@mkdir -p $(dir $@)
	$(PNGTO) -p "3210" $(filter-out $<,$^) $@

$(OBJDIR)/%H16_gfx.bin: $(PNGTO) $(RESDIR)/%H16.png
	@mkdir -p $(dir $@)
	$(PNGTO) -p "3210" -H 16 $(filter-out $<,$^) $@

$(OBJDIR)/%H24_gfx.bin: $(PNGTO) $(RESDIR)/%H24.png
	@mkdir -p $(dir $@)
	$(PNGTO) -p "3210" -H 24 $(filter-out $<,$^) $@

$(OBJDIR)/%H32_gfx.bin: $(PNGTO) $(RESDIR)/%H32.png
	@mkdir -p $(dir $@)
	$(PNGTO) -p "3210" -H 32 $(filter-out $<,$^) $@

$(OBJDIR)/%_pal.bin: $(PNGTO) $(RESDIR)/%.png
	@mkdir -p $(dir $@)
	$(PNGTO) -c 0000BBB0GGG0RRR0 --num-colors=16 $(filter-out $<,$^) $@
