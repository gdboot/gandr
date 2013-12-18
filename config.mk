# Configuration file for Gandalf build.

# The prefix for the cross-compiler and binutils.
PREFIX   ?= ./tools/bin

# The platform and architecture.
# Allowed architectures for each platform: 
#     [bios] arch: x86, x86_64.
PLATFORM ?= bios
ARCH     ?= x86

# Boot device.
# Allowed boot devices:
#     [bios] device: eltorito, floppy, pxe.
BOOT_DEVICE ?= pxe

# C flags.
CFLAGS   := -O2
