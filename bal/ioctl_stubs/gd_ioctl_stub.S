/* Copyright © 2013-2014, Owen Shepherd
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED AS IS AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

.global gd_ioctl

#ifdef __i386__
gd_ioctl:
	mov 4(%esp), %eax
   jmp *(%eax)

#elif defined(__amd64__)

gd_ioctl:
	jmp *(%rsi)


#elif defined(__arm__)

	ldr pc, [r0]

#elif defined(__aarch64__)

gd_ioctl:
    ldr x16, [x0]
    br  x16

#else

#error You need to define an ioctl stub for this arch

#endif
