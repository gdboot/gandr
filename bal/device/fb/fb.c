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
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static void fb_clear_buffers(struct fb_dev *dev)
{
    dev->cur_x = dev->cur_y = 0;
    if (dev->back_buffer) {
        dev->back_buffer_start = 0;
        memset(dev->back_buffer, 0, dev->cur_mode.width * dev->cur_mode.height * dev->cur_mode.depth / 8);
    }

    for (int i = 0; i < dev->cur_mode.height; i++)
        memset(dev->front_buffer + (i * dev->cur_mode.bytes_per_scanline), 0,
               dev->cur_mode.width * dev->cur_mode.depth / 8);
}

static void fb_scroll(struct fb_dev *dev)
{
    if (dev->back_buffer) {
        memset(dev->back_buffer + (dev->back_buffer_start * font_height), 0,
               dev->cur_mode.width * dev->cur_mode.depth / 8);
        dev->back_buffer_start = (dev->back_buffer_start + 1) % dev->max_height;
    } else {
        uint8_t *dest = dev->front_buffer;
        int char_line = (font_height * dev->cur_mode.bytes_per_scanline); 
        for (unsigned int i = 0; i < dev->max_height - 1; i++) {
            for (int j = 0; j < font_height; j++, dest += dev->cur_mode.bytes_per_scanline)
                memcpy(dest, dest + char_line, dev->cur_mode.width * dev->cur_mode.depth / 8);
        }

        for (int j = 0; j < font_height; j++, dest += dev->cur_mode.bytes_per_scanline)
            memset(dest, 0, dev->cur_mode.width * dev->cur_mode.depth / 8);
    }
}

static void fb_write_char_32(struct fb_dev *dev, char c)
{
    uint32_t *dest, bytes_per_scanline;
    if (dev->back_buffer) {
        bytes_per_scanline = dev->cur_mode.width * 4;
        dest = (uint32_t*) dev->back_buffer;
    } else {
        bytes_per_scanline = dev->cur_mode.bytes_per_scanline;
        dest = (uint32_t*) dev->front_buffer;  
    }

    dest += (dev->cur_y * font_height * bytes_per_scanline / 4);
    dest += (dev->cur_x * font_width);
    uint8_t *char_font = &font_data[(uint8_t)c * font_height];
    for (int i = 0; i < font_height; i++) {
        for (int j = 0; j < 8; j++)
            dest[j] = ((*char_font >> (7 - j)) & 1) ? 0xFFFFFFFF : 0;

        char_font++;
        dest += bytes_per_scanline / 4;
    }
}

void fb_write_char(struct fb_dev *dev, char c)
{
    if (dev->cur_mode.depth == 32)
        fb_write_char_32(dev, c);

    if (++dev->cur_x == dev->max_width) {
        dev->cur_x = 0;
        if (++dev->cur_y == dev->max_height)
            fb_scroll(dev);
    }
}

GD_BEGIN_IOCTL_MAP(struct fb_dev *, fb_ioctl)
//    GD_MAP_WRITE_IOCTL(fb_write)
GD_END_IOCTL_MAP()

void fb_init(struct fb_dev *dev, struct mode_info mode)
{
    dev->dev.ioctl = fb_ioctl;

    memcpy(&dev->cur_mode, &mode, sizeof mode); 

    assert(font_width == 8);
    dev->max_width = dev->cur_mode.width / font_width;
    dev->max_height = dev->cur_mode.height / font_height;

    //dev->back_buffer = (uint8_t *) malloc(dev->cur_mode.width * dev->cur_mode.height * dev->cur_mode.depth / 8);
    dev->back_buffer = 0;
    dev->front_buffer = (uint8_t *) (uintptr_t) dev->cur_mode.lfb_address;
    
    fb_clear_buffers(dev);
}
