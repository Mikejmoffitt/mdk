# Project meta configuration
PROJECT_NAME := blank-mdk-project
MDKROOT := mdk/mdk

# Project directories.
SRCDIR := src
RESDIR := res
OBJDIR := obj

# Sources.
SOURCES_C := $(shell find $(SRCDIR) -type f -name '*.c')
SOURCES_CPP := $(shell find $(SRCDIR) -type f -name '*.cpp')
SOURCES_ASM := $(shell find $(SRCDIR) -type f -name '*.s')

# Target configuration.
# TARGET_SYSTEM:
#   * MDK_TARGET_MD  ; Default. Use this for megadrive
#   * MDK_TARGET_C2  ; Use this to target the Sega System C/C2 PCB.
# In addition, you may define MDK_C2_ADPCM with the filename of the ADPCM ROM.
TARGET_SYSTEM := MDK_TARGET_MD

# Any additional CFLAGS/CPPFLAGS/ASFLAGS additions or build targets may go here.
# Please read about EXTERNAL_DEPS and EXTERNAL_ARTIFACTS in readme.md.

include $(MDKROOT)/md-rules.mk
