#include <bal/gio.h>

void gio_write_index(size_t size, gio_addr base, size_t index, size_t value)
{
    switch(size) {
    case 8:     gio_write8_index (base, index, value); return;
    case 16:    gio_write16_index(base, index, value); return;
    case 32:    gio_write32_index(base, index, value); return;
    default:    for(;;); // todo - abort
    }
}

size_t gio_read_index(size_t size, gio_addr base, size_t index)
{
    switch(size) {
    case 8:     return gio_read8_index (base, index);
    case 16:    return gio_read16_index(base, index); 
    case 32:    return gio_read32_index(base, index);
    default:    for(;;); // todo - abort
    }
}

void gio_write(size_t size, gio_addr addr, size_t value)
{ gio_write_index(size, addr, 0, value); }

size_t gio_read(size_t size, gio_addr addr)
{ return gio_read_index(size, addr, 0); }