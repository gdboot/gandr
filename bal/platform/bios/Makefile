# Include the config and rules.
include config.mk
include rules.mk

# List of CFLAGS.
CFLAGS += -std=c99 -Wall -Wextra -nostdlib -ffreestanding -lgcc $(CCFLAGS)

# Pre-processor flags.
CPPFLAGS := -D$(shell echo "BD_$(BOOT_DEVICE)" | tr '[:lower:]' '[:upper:]')

# Stage 1.
STAGE1_SRC := $(shell ls bal/platform/$(PLATFORM)/stage1/*.S) \
			  $(shell find bal/platform/$(PLATFORM)/stage1/$(BOOT_DEVICE) -type f -name "*.S")

# Get the object files.
STAGE1_OBJ := $(patsubst %.S,%.o,$(STAGE1_SRC))

# Get the dependancies.
DEP := $(patsubst %.o,%.d,$(STAGE1_OBJ))

# Make related files.
MAKEDEPS := rules.mk config.mk bal/platform/$(PLATFORM)/Makefile

# The binaries to produce.
all: stage1

# List phony targets.
.PHONY: all clean

stage1: $(STAGE1_OBJ) bal/platform/$(PLATFORM)/stage1/link.ld
	$(HOSTLD) $(STAGE1_OBJ) -Tbal/platform/$(PLATFORM)/stage1/link.ld -o $@

# Include dependancy files.
-include $(DEP)

# Clean.
clean: 
	-$(RM) $(wildcard $(STAGE1_OBJ) $(DEP)) stage1

# CC.
%.o: %.c $(MAKEDEPS)
	$(HOSTCC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

# AS.
%.o: %.S $(MAKEDEPS)
	$(HOSTCC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@
