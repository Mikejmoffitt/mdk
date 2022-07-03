# Project meta configuration
PROJECT_NAME := hello-world
MDKROOT := ../../mdk

# Project directories.
SRCDIR := src
RESDIR := res
OBJDIR := obj

# Sources.
SOURCES_C := $(shell find $(SRCDIR)/ -type f -name '*.c')
SOURCES_ASM := $(shell find $(SRCDIR)/ -type f -name '*.s')
RESOURCES_LIST := $(shell find $(RESDIR)/ -type f -name '*')

# Compiler flags.
CFLAGS := -I$(SRCDIR) -I$(OBJDIR) -I$(MDKSRCDIR)

include $(MDKROOT)/md-rules.mk

# Any additional build targets may go here.
# Please read about EXTERNAL_DEPS and EXTERNAL_ARTIFACTS in readme.md.
