# Gandr for ARM Trusted Firmware

This platform is designed to be the userspace bootloader started after ARM
Trusted Firmware on non-UEFI platforms (if your platform uses ARM Trusted
Firmware and UEFI, this is irrelevant - use a UEFI build of Gandr instead)

The BAL produced, gd_atf[32|64].elf, is a relocatable ELF file which can be
located wherever appropriate in memory. It is designed to be placed "on disk"
(i.e. in your device's /boot filesystem), to facilitate upgrades. A small loader
is then placed inside your ATF firmware image package as BL3-3, which loads a
device tree binary and the BAL from disk.

Presently, only the ARM Foundation Virtual Platform is supported. Other
platforms can be supported in the future. Porting to Juno should be reasonably
easy.

A reference BL3-3, called Kickstart! is provided, which uses semihosting to load
the BAL and DTB from host disk (in the current working directory). This could
be relatively easily adapted to support hardware platforms.

# BAL Interface Conventions
The BAL shall always be an ELF dynamic (ET_DYN) binary of the appropriate
architecture, bitness and endiannes.

The BAL shall define a dynamic entry of 'DT_GANDR_ATFKICKSTART_VERSION'
(0x67646b21) defining the maximum interface version supported.

For the version 0 interface, the register contents on entry to the BAL shall be
the following:

(aarch32/aarch64 register name)
* r0/w0 - 0x67646b21 ('gdk!') - magic constant identifying that the ATF Kickstart! interface is in use
* r1/w1 - Interface version number (0)
* r2/x2 - pointer to device tree binary

The BAL will obey the memory and memory reservation declarations inside the
device tree binary. In addition, it will note the memory occupied by its' own
image and by the device tree binary. It will configure its' own stack. It is
expected that the BL3-3 will not be marked as being in used memory and therefore
its memory may be relalocated.

The processor shall be in either EL1 or EL2.
