# Include the config and rules.
include config.mk
include rules.mk

# The default target.
all:
	$(MAKE) -f bal/platform/$(PLATFORM)/Makefile

# List phony targets.
.PHONY: all clean

# Clean.
clean: 
	$(MAKE) -f bal/platform/$(PLATFORM)/Makefile clean
