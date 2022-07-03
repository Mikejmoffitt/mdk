MAKE := make
PROJECTS := $(wildcard */.)

.PHONY: all $(PROJECTS)

all: $(PROJECTS)
$(PROJECTS):
	$(MAKE) -C $@
