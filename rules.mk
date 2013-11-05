# Rules for Gandalf.

# Get the tool prefix.
ifeq ($(ARCH), x86)
    TOOLPREFIX := $(PREFIX)/i386-elf

    HOSTCC := $(TOOLPREFIX)-gcc
    HOSTAS := $(TOOLPREFIX)-as
    HOSTLD := $(TOOLPREFIX)-ld
    HOSTOBJCOPY := $(TOOLPREFIX)-objcopy
else
    $(error ARCH set to $(ARCH), which is not a recognized architecture)
endif