/*
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (C) 2013 Owen Shepherd - http://owenshepherd.net/
 */
/* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/* Linker script for Gandr on TI AM33XX SoCs. Derived from the Starterware
 * linkerscript
 */

MEMORY
{
    TI_HEADER :   o = 0x402F03F8,  l = 0x08          /* 8 byte image header */
    IRAM_MEM  :   o = 0x402F0400,  l = 0x1FBFF       /* 127k internal Memory */
}

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

SECTIONS
{
    .ti_hdr : {
        LONG(_image_size);
        LONG(_entry);
    } >TI_HEADER

    .text :
    {
        . = ALIGN(4);
        *(entry)

        /* Force us past the first 64kB of SRAM
         * The debug TAP can't see into it - OpenOCD bug perhaps?
         */
        . += 65536;
        *(.text*)
        *(.rodata*)
    } >IRAM_MEM

    .data :
    {
        . = ALIGN(4);
        *(.data*)
        _end = .; 
    } >IRAM_MEM

    .bss :
    {
        . = ALIGN(4);
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        _bss_end = .;
    } >IRAM_MEM

    _image_size = _end - _entry;
}

