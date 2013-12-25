/*
 * Stage 1.5 (disk independent, firmware dependent) initialization code.
 */

.code16
.section header

binary_header:
    .long _start

/*
 * Entry point.
 */
.global _start
_start:
