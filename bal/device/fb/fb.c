/* Copyright © 2014, Shikhin Sethi
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <gd_bal.h>
#include <bal/device/fb.h>
#include <bal/device/font_data.h>
#include <arch/string.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static inline uint32_t fb_get_pixel_color(struct fb_dev *dev, uint8_t red, uint8_t green, uint8_t blue)
{
    red >>= (8 - dev->cur_mode.red_mask_size);
    green >>= (8 - dev->cur_mode.green_mask_size);
    blue >>= (8 - dev->cur_mode.blue_mask_size);
    return (red << dev->cur_mode.red_field_pos) |
           (green << dev->cur_mode.green_field_pos) |
           (blue << dev->cur_mode.blue_field_pos);
}

static inline void fb_reset_foreground_color(struct fb_dev *dev)
{
    dev->foreground.red = dev->foreground.green = dev->foreground.blue = 0xFF;
    dev->foreground.pixel = fb_get_pixel_color(dev, 0xFF, 0xFF, 0xFF);
}

static inline void fb_reset_background_color(struct fb_dev *dev)
{
    dev->background.red = dev->background.green = dev->background.blue = 0x00;
    dev->background.pixel = 0;
}

static inline void memset_pixel(struct fb_dev *dev, void *dest, uint32_t pixel, size_t n)
{
    uint8_t *d = dest;
    switch (dev->cur_mode.depth) {
        case 8:
            memset8(dest, pixel, n); break;
        case 16:
            memset16(dest, pixel, n); break;
        case 24:
            for (size_t i = 0; i < n; i++) {
                d[3 * i + 0] = pixel & 0xFF;
                d[3 * i + 1] = (pixel >> 8) & 0xFF;
                d[3 * i + 2] = (pixel >> 16) & 0xFF;
            }

            break;
        case 32:
            memset32(dest, pixel, n); break;
    }
}

static inline void memcpy_pixel(struct fb_dev *dev, void* dest, const void* src, size_t n)
{
    uint8_t *d = dest; const uint8_t* s = src;
    switch (dev->cur_mode.depth) {
        case 8:
            memcpy8(dest, src, n); break;
        case 16:
            memcpy16(dest, src, n); break;
        case 24:
            for (size_t i = 0; i < n; i++) {
                d[3 * i + 0] = s[3 * i + 0];
                d[3 * i + 1] = s[3 * i + 1];
                d[3 * i + 2] = s[3 * i + 2];
            }

            break;
        case 32:
            memcpy32(dest, src, n); break;
    }
}

static void fb_render(struct fb_dev *dev)
{
    if (!dev->back_buffer || dev->front_dirty_end <= dev->front_dirty_start) return;

    int start = (dev->front_dirty_start + dev->back_buffer_start) % dev->max_height,
             end = (dev->front_dirty_end + dev->back_buffer_start) % dev->max_height;
    if (dev->flags & CONTIG_SCANLINES) {
        // No extra bytes, just copy fully.
        size_t char_line = font_height * dev->cur_mode.bytes_per_scanline;
        if (start < end) {
            // Doesn't wrap around.
            memcpy_pixel(dev, (uint8_t*) dev->front_buffer + dev->front_dirty_start * char_line,
                         (uint8_t*) dev->back_buffer + start * char_line,
                         (end - start) * char_line * 8 / dev->cur_mode.depth);
        } else {
            // Wraps around.
            // To end.
            memcpy_pixel(dev, (uint8_t*) dev->front_buffer + dev->front_dirty_start * char_line,
                         (uint8_t*) dev->back_buffer + start * char_line,
                         (dev->max_height - dev->back_buffer_start) * char_line * 8 / dev->cur_mode.depth);
            // From beginning.
            memcpy_pixel(dev, (uint8_t*) dev->front_buffer +
                         (dev->front_dirty_start + dev->max_height - dev->back_buffer_start) * char_line,
                         (uint8_t*) dev->back_buffer,
                         end * char_line * 8 / dev->cur_mode.depth);
        }
    } else {
        // Copy per scanline.
        size_t char_line = dev->cur_mode.width * dev->cur_mode.depth / 8;
        if (start < end) {
            // If it doesn't wrap around in the circular buffer,
            for (int i = start; i < end; i++)
                // Copy pixel line-wise for each charline.
                for (int j = 0; j < font_height; j++)
                    memcpy_pixel(dev, (uint8_t*) dev->front_buffer +
                                       ((dev->front_dirty_start + i) * font_height + j) * dev->cur_mode.bytes_per_scanline,
                                 (uint8_t*) dev->back_buffer + ((start + i) * font_height + j) * char_line,
                                 char_line * 8 / dev->cur_mode.depth);
        } else {
            // If it does wrap around, copy first to the end.
            for (unsigned i = 0; i < (dev->max_height - dev->back_buffer_start); i++)
                for (int j = 0; j < font_height; j++)
                    memcpy_pixel(dev, (uint8_t*) dev->front_buffer +
                                       ((dev->front_dirty_start + i) * font_height + j) * dev->cur_mode.bytes_per_scanline,
                                 (uint8_t*) dev->back_buffer + ((start + i) * font_height + j) * char_line,
                                 char_line * 8 / dev->cur_mode.depth);

            // Then from beginning to left over portion.
            for (int i = 0; i < end; i++)
                for (int j = 0; j < font_height; j++)
                    memcpy_pixel(dev, (uint8_t*) dev->front_buffer +
                                       ((dev->front_dirty_start + dev->max_height - dev->back_buffer_start + i) * font_height + j)
                                    * dev->cur_mode.bytes_per_scanline,
                                 (uint8_t*) dev->back_buffer + (i * font_height + j) * char_line,
                                 char_line * 8 / dev->cur_mode.depth);
        }
    }
}

static void fb_clear_buffers(struct fb_dev *dev)
{
    dev->cur_x = dev->cur_y = 0;
    if (dev->back_buffer) {
        dev->back_buffer_start = 0;
        memset_pixel(dev, dev->back_buffer,
                     dev->background.pixel, dev->cur_mode.width * dev->max_height * font_height);
    }

    if (dev->flags & CONTIG_SCANLINES)
        memset_pixel(dev, dev->front_buffer, dev->background.pixel, dev->cur_mode.width * dev->cur_mode.height);
    else for (int i = 0; i < dev->cur_mode.height; i++)
        memset_pixel(dev, dev->front_buffer + (i * dev->cur_mode.bytes_per_scanline), dev->background.pixel,
               dev->cur_mode.width);

    dev->front_dirty_start = dev->front_dirty_end = 0;
}

static void fb_line_dirty(struct fb_dev *dev, unsigned line)
{
    // Include line as dirty.
    if (dev->front_dirty_start > line) dev->front_dirty_start = line;
    if (dev->front_dirty_end <= line) dev->front_dirty_end = line + 1;
}

static void fb_scroll(struct fb_dev *dev)
{
    if (dev->back_buffer) {
        // Move start of circular buffer one ahead.
        memset_pixel(dev, (uint8_t*) dev->back_buffer +
                           (dev->back_buffer_start * font_height * dev->cur_mode.width * dev->cur_mode.depth / 8),
                     dev->background.pixel, dev->cur_mode.width * font_height);
        dev->back_buffer_start = (dev->back_buffer_start + 1) % dev->max_height;
        dev->front_dirty_start = 0; dev->front_dirty_end = dev->max_height;
    } else {
        uint8_t *dest = dev->front_buffer;
        size_t char_line = (font_height * dev->cur_mode.bytes_per_scanline);

        if (dev->flags & CONTIG_SCANLINES) {
            memcpy_pixel(dev, dest, dest + char_line, dev->cur_mode.width * (dev->cur_mode.height - font_height));

            dest += dev->cur_mode.width * (dev->cur_mode.height - font_height) * dev->cur_mode.depth / 8;
            // Clear last line.
            memset_pixel(dev, dest, dev->background.pixel, dev->cur_mode.width * font_height);
        } else {
            for (unsigned int i = 0; i < dev->max_height - 1; i++) {
                for (int j = 0; j < font_height; j++, dest += dev->cur_mode.bytes_per_scanline)
                    memcpy_pixel(dev, dest, dest + char_line, dev->cur_mode.width);
            }

            for (int j = 0; j < font_height; j++, dest += dev->cur_mode.bytes_per_scanline)
                memset_pixel(dev, dest, dev->background.pixel, dev->cur_mode.width);
        }
    }
}

static void fb_write_char_8(struct fb_dev *dev, char c)
{
    uint8_t *dest; uint32_t bytes_per_scanline; int y;
    if (dev->back_buffer) {
        bytes_per_scanline = dev->cur_mode.width;
        dest = (uint8_t*) dev->back_buffer;
        y = (dev->cur_y + dev->back_buffer_start) % dev->max_height;
    } else {
        bytes_per_scanline = dev->cur_mode.bytes_per_scanline;
        dest = (uint8_t*) dev->front_buffer;
        y = dev->cur_y;
    }

    dest += (y * font_height * bytes_per_scanline);
    dest += (dev->cur_x * font_width);
    uint8_t *char_font = &font_data[(uint8_t)c * font_height];
    for (int i = 0; i < font_height; i++) {
        for (int j = 0; j < 8; j++)
            dest[j] = ((*char_font >> (7 - j)) & 1) ?
                        dev->foreground.pixel : dev->background.pixel;

        char_font++;
        dest += bytes_per_scanline;
    }
}

static void fb_write_char_16(struct fb_dev *dev, char c)
{
    uint16_t *dest; uint32_t bytes_per_scanline; int y;
    if (dev->back_buffer) {
        bytes_per_scanline = dev->cur_mode.width * 2;
        dest = (uint16_t*) dev->back_buffer;
        y = (dev->cur_y + dev->back_buffer_start) % dev->max_height;
    } else {
        bytes_per_scanline = dev->cur_mode.bytes_per_scanline;
        dest = (uint16_t*) dev->front_buffer;
        y = dev->cur_y;
    }

    dest += (y * font_height * bytes_per_scanline / 2);
    dest += (dev->cur_x * font_width);
    uint8_t *char_font = &font_data[(uint8_t)c * font_height];
    for (int i = 0; i < font_height; i++) {
        for (int j = 0; j < 8; j++)
            dest[j] = ((*char_font >> (7 - j)) & 1) ?
                        dev->foreground.pixel : dev->background.pixel;

        char_font++;
        dest += bytes_per_scanline / 2;
    }
}

static void fb_write_char_24(struct fb_dev *dev, char c)
{
    uint8_t *dest; uint32_t bytes_per_scanline; int y;
    if (dev->back_buffer) {
        bytes_per_scanline = dev->cur_mode.width * 3;
        dest = (uint8_t*) dev->back_buffer;
        y = (dev->cur_y + dev->back_buffer_start) % dev->max_height;
    } else {
        bytes_per_scanline = dev->cur_mode.bytes_per_scanline;
        dest = (uint8_t*) dev->front_buffer;
        y = dev->cur_y;
    }

    dest += (y * font_height * bytes_per_scanline);
    dest += (dev->cur_x * font_width * 3);
    uint8_t *char_font = &font_data[(uint8_t)c * font_height];
    for (int i = 0; i < font_height; i++) {
        for (int j = 0; j < 8; j++) {
            uint32_t pixel = ((*char_font >> (7 - j)) & 1) ?
                              dev->foreground.pixel : dev->background.pixel;
            dest[j * 3] = pixel;
            dest[j * 3 + 1] = pixel >> 8;
            dest[j * 3 + 2] = pixel >> 16;
        }

        char_font++;
        dest += bytes_per_scanline;
    }
}

static void fb_write_char_32(struct fb_dev *dev, char c)
{
    uint32_t* dest, bytes_per_scanline; int y;
    if (dev->back_buffer) {
        bytes_per_scanline = dev->cur_mode.width * 4;
        dest = (uint32_t*) dev->back_buffer;
        y = (dev->cur_y + dev->back_buffer_start) % dev->max_height;
    } else {
        bytes_per_scanline = dev->cur_mode.bytes_per_scanline;
        dest = (uint32_t*) dev->front_buffer;
        y = dev->cur_y;
    }

    dest += (y * font_height * bytes_per_scanline / 4);
    dest += (dev->cur_x * font_width);
    uint8_t *char_font = &font_data[(uint8_t)c * font_height];
    for (int i = 0; i < font_height; i++) {
        for (int j = 0; j < 8; j++)
            dest[j] = ((*char_font >> (7 - j)) & 1) ?
                       dev->foreground.pixel : dev->background.pixel;

        char_font++;
        dest += bytes_per_scanline / 4;
    }
}

static void fb_write_char(struct fb_dev *dev, char c)
{
    dev->fb_write_char_depth(dev, c);
    fb_line_dirty(dev, dev->cur_y);
    if (++dev->cur_x == dev->max_width) {
        dev->cur_x = 0;
        if (dev->cur_y == dev->max_height - 1) fb_scroll(dev);
        else dev->cur_y++;
    }
}

static int fb_write(struct fb_dev *dev, const void *buf, size_t sz, size_t *wrote)
{
    const unsigned char *cbuf = buf;

    for (size_t i = 0; i < sz; i++) {
        unsigned char c = cbuf[i];

        switch (c) {
            case '\r':
                continue;
            case '\n':
                dev->cur_x = 0;
                if (dev->cur_y == dev->max_height - 1) fb_scroll(dev);
                else dev->cur_y++;
                break;
            case '\t':
                dev->cur_x += 8 - (dev->cur_x % 8);
                if (dev->cur_x >= dev->max_width) dev->cur_x = dev->max_width - 1;
                break;
            default:
                fb_write_char(dev, c);
        }
    }

    // If there's a dirty backbuffer, copy to frontbuffer.
    fb_render(dev);
    return *wrote = sz, 0;
}

GD_BEGIN_IOCTL_MAP(struct fb_dev *, fb_ioctl)
    GD_MAP_WRITE_IOCTL(fb_write)
GD_END_IOCTL_MAP()

void fb_init(struct fb_dev *dev, struct mode_info mode)
{
    dev->dev.ioctl = fb_ioctl;

    memcpy(&dev->cur_mode, &mode, sizeof mode); 

    assert(font_width == 8);
    dev->max_width = dev->cur_mode.width / font_width;
    dev->max_height = dev->cur_mode.height / font_height;

    dev->back_buffer = (uint8_t *) malloc(dev->cur_mode.width * dev->max_height * font_height * dev->cur_mode.depth / 8);
    dev->front_buffer = (uint8_t *) (uintptr_t) dev->cur_mode.lfb_address;

    switch (dev->cur_mode.depth) {
        case 8:
            dev->fb_write_char_depth = &fb_write_char_8;
            break;
        case 16:
            dev->fb_write_char_depth = &fb_write_char_16;
            break;
        case 24:
            dev->fb_write_char_depth = &fb_write_char_24;
            break;
        case 32:
            dev->fb_write_char_depth = &fb_write_char_32;
            break;
    }

    dev->flags = 0;
    if (dev->cur_mode.bytes_per_scanline == (unsigned) (dev->cur_mode.width * dev->cur_mode.depth / 8))
        dev->flags |= CONTIG_SCANLINES;

    fb_reset_background_color(dev);
    fb_reset_foreground_color(dev);
    fb_clear_buffers(dev);
}
