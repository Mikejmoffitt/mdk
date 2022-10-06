# Configuration
PROJECT_NAME := move_podge

MDKROOT := ../../mdk

SRCDIR := src
RESDIR := res
OBJDIR := obj
PNGDIR := png

SOURCES_C := $(shell find $(SRCDIR) -type f -name '*.c')

# Experimental PNG rules - in this example, the `res` directory contains
# artifacts from automatic image conversion.
IMAGES_PNG := $(shell find $(PNGDIR) -type f -name '*.png')
CONVERTED_CHR := $(patsubst $(PNGDIR)/%.png,$(RESDIR)/%_gfx.bin,$(IMAGES_PNG))
CONVERTED_PAL := $(patsubst $(PNGDIR)/%.png,$(RESDIR)/%_pal.bin,$(IMAGES_PNG))

EXTERNAL_DEPS := gen_graphics
EXTERNAL_ARTIFACTS := $(CONVERTED_CHR) $(CONVERTED_PAL)

include $(MDKROOT)/md-rules.mk

gen_graphics: $(CONVERTED_CHR) $(CONVERTED_PAL)

$(RESDIR)/%_gfx.bin: $(PNGTO) $(PNGDIR)/%.png
	@mkdir -p $(dir $@)
	$(PNGTO) -p "3210" -i $(filter-out $<,$^) -o $@

$(RESDIR)/%H16_gfx.bin: $(PNGTO) $(PNGDIR)/%H16.png
	@mkdir -p $(dir $@)
	$(PNGTO) -p "3210" -H 16 -i $(filter-out $<,$^) -o $@

$(RESDIR)/%H24_gfx.bin: $(PNGTO) $(PNGDIR)/%H24.png
	@mkdir -p $(dir $@)
	$(PNGTO) -p "3210" -H 24 -i $(filter-out $<,$^) -o $@

$(RESDIR)/%H32_gfx.bin: $(PNGTO) $(PNGDIR)/%H32.png
	@mkdir -p $(dir $@)
	$(PNGTO) -p "3210" -H 32 -i $(filter-out $<,$^) -o $@

$(RESDIR)/%_pal.bin: $(PNGTO) $(PNGDIR)/%.png
	@mkdir -p $(dir $@)
	$(PNGTO) -c 0000BBB0GGG0RRR0 --num-colors=16 -i $(filter-out $<,$^) -o $@

