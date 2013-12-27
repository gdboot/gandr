/*
 * Stage 1.5 (disk independent, firmware dependent) initialization code.
 */

.code16
.section header

/*
 * The binary header to parse the file.
 */
binary_header:
    .long 0x004C4142    /* BAL identifier. */
    .long _start        /* Entry point. */
    .long 0x17000       /* Start of file. */
    .long _end          /* End of file. */
    .long _bss_start    /* Start of BSS. */
    .long _bss_end      /* End of BSS. */
    .long 0x00000000    /* Reserved for CRC32. */

/*
 * Entry point.
 */
.global _start
_start:
