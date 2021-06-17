# Configuration
OUTPUT_FILE := move-podge
OUTPUT_EXT := gen
OUTPUT_VERSION := wip

BASE_FRAMEWORK_DIR := ../..

CPUTYPE := 68000
SRCDIR := src
COMMONSRCDIR := $(BASE_FRAMEWORK_DIR)/common/src
RESDIR := res
OBJDIR := obj
UTILDIR := $(BASE_FRAMEWORK_DIR)/util
CFLAGS := -I$(SRCDIR) -I$(OBJDIR) -I$(COMMONSRCDIR) -Wall -Wextra -O2 -std=c11 -Wno-unused-function #-Werror
HOSTCFLAGS := -Os -std=c11
LDSCRIPT := $(BASE_FRAMEWORK_DIR)/common/md.ld
SOURCES_C := $(shell find $(SRCDIR)/ -type f -name '*.c')
SOURCES_ASM := $(shell find $(SRCDIR)/ -type f -name '*.s')
COMMONSOURCES_C := $(shell find $(COMMONSRCDIR)/ -type f -name '*.c')
COMMONSOURCES_ASM := $(shell find $(COMMONSRCDIR)/ -type f -name '*.s')
RESOURCES_LIST := $(shell find $(RESDIR)/ -type f -name '*.bin')
LIBS :=

include $(BASE_FRAMEWORK_DIR)/common/md-rules.mk
