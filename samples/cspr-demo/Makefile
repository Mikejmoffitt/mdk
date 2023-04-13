# Project meta configuration
PROJECT_NAME := cspr-demo
MDKROOT := ../../mdk

# Project directories.
SRCDIR := src
RESDIR := res
OBJDIR := obj
PNGDIR := png

# Sources.
SOURCES_C := $(shell find $(SRCDIR) -type f -name '*.c')

EXTERNAL_DEPS := convert_graphics
EXTERNAL_ARTIFACTS := $(RESDIR)/cirno.csp $(RESDIR)/count.csp

include $(MDKROOT)/md-rules.mk

convert_graphics: $(EXTERNAL_ARTIFACTS)

$(RESDIR)/cirno.csp: $(PNG2CSP)
	$(PNG2CSP) png/cirno.png 80 80 $@ "Cirno Laugh"

$(RESDIR)/count.csp: $(PNG2CSP)
	$(PNG2CSP) png/count.png 64 64 $@ "Count Anim"
