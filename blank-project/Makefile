# Project meta configuration
PROJECT_NAME := blank-mdk-project
MDKROOT := mdk/mdk

# Project directories.
SRCDIR := src
RESDIR := res
OBJDIR := obj

# Sources.
SOURCES_C := $(shell find $(SRCDIR)/ -type f -name '*.c')
SOURCES_ASM := $(shell find $(SRCDIR)/ -type f -name '*.s')
RESOURCES_LIST := $(shell find $(RESDIR)/ -type f -name '*')

include $(MDKROOT)/md-rules.mk

# Any additional CFLAGS/ASFLAGS ammendments and build targets may go here.
# Please read about EXTERNAL_DEPS and EXTERNAL_ARTIFACTS in readme.md.
