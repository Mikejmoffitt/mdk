# Project meta configuration
PROJECT_NAME := system-c2-demo
MDKROOT := ../../mdk

# Project directories.
SRCDIR := src
RESDIR := res
OBJDIR := obj

# Sources.
SOURCES_C := $(shell find $(SRCDIR) -type f -name '*.c')

# Target configuration.
# TARGET_SYSTEM:
#   * MDK_TARGET_MD
# The default - use this for Mega Drive.
#   * MDK_TARGET_C2
# Use this to target the Sega System C arcade PCB.
# In addition, you may define MDK_C2_ADPCM with the filename of the ADPCM ROM.
TARGET_SYSTEM := MDK_TARGET_C2

MDK_C2_ADPCM := adpcm/puyo2.bin

include $(MDKROOT)/md-rules.mk
