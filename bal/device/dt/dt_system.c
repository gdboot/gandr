#include <bal/device/dt.h>
#include <bal/misc.h>
#include <bal/mmap.h>
#include <libfdt.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

void *system_fdt;
static dt_node_t dt_root;

static void print_dtval(const char *val, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if(isprint(val[i]))
            putc(val[i], stdout);
        else
            printf("\\x%02x", (unsigned) val[i]);
    }
}

static dt_node_t add_device_fdt(dt_node_t parent, void *fdt, int fdt_node, int depth)
{
    const char *name = fdt_get_name(fdt, fdt_node, NULL);
    printf("%*cEnumerating \"%s\"\n", depth, ' ', name);
    dt_node_t n = dt_node_alloc(parent, name);
    if (!n)
        panic("Out of memory enumerating node \"%s\"\n", name);

    for (int propoff = fdt_first_property_offset(fdt, fdt_node);
             propoff >= 0;
             propoff = fdt_next_property_offset(fdt, propoff)) {
        int len;
        const struct fdt_property *fdt_prop =
            fdt_get_property_by_offset(fdt, propoff, &len);

        if (!fdt_prop) {
            panic("Error getting property of \"%s\": %s\n",
                name, fdt_strerror(len));
        }

        const char *prop_name = fdt_string(fdt, fdt32_to_cpu(fdt_prop->nameoff));

        printf("%*c \"%s\" = \"", depth, ' ', prop_name);
        print_dtval(fdt_prop->data, len);
        printf("\"\n");

        if (!dt_node_set_property(n, prop_name, fdt_prop->data, len))
            panic("Out of memory allocating \"%s\":\"%s\"",
                name, prop_name);
    }

    for (int suboff = fdt_first_subnode(fdt, fdt_node);
             suboff != -FDT_ERR_NOTFOUND;
             suboff = fdt_next_subnode(fdt, suboff)) {
        add_device_fdt(n, fdt, suboff, depth + 2);
    }

    return n;
}

void dt_platform_init_fdt(void *fdt)
{
    int rv;
    system_fdt = fdt;

    printf("fdt_platform_init %p\n", fdt);

    // Check header
    if ((rv = fdt_check_header(fdt))) {
        panic("Bad FDT: %s\n", fdt_strerror(rv));
    }

    // Process memory reservations
    printf("rsv %p\n", fdt);
    int num_rsv = fdt_num_mem_rsv(fdt);
    for (int i = 0; i < num_rsv; i++) {
        printf("get\n");
        gd_memory_map_entry ent;
        ent.type = gd_reserved_memory_type;
        ent.attributes = 0;
        fdt_get_mem_rsv(fdt, i, &ent.physical_start, &ent.size);
        printf("got\n");
        ent.virtual_start = ent.physical_start;

        printf("Adding reserved memory region: %" PRIX64 " len=%" PRIX64 "\n",
            ent.physical_start, ent.size);

        mmap_add_entry(ent);
    }

    // Process root note memory entries
    printf("root\n");
    int root = fdt_next_node(fdt, -1, NULL);

    unsigned addr_cells = fdt_address_cells(fdt, root);
    unsigned size_cells = fdt_size_cells(fdt, root);
    unsigned cells      = addr_cells + size_cells;

    int mem_offs = fdt_subnode_offset(fdt, root, "memory");
    if (mem_offs < 0) {
        panic("Unable to locate memory node (%s)\n", fdt_strerror(mem_offs));
    }

    int memlen;
    const uint32_t *p = fdt_getprop(system_fdt, mem_offs, "reg", &memlen);
    for (int i = 0; i < memlen / 4; i+= cells) {
        uint64_t base_addr = 0, base_size = 0;
        if (addr_cells == 1) {
            base_addr = fdt32_to_cpu(p[i]);
        } else if (addr_cells == 2) {
            base_addr = fdt64_to_cpu(((uint64_t) p[i + 1]) << 32 | p[i]);
        }

        if (size_cells == 1) {
            base_size = fdt32_to_cpu(p[i + addr_cells]);
        } else if (size_cells == 2) {
            base_size = fdt64_to_cpu(((uint64_t) p[i + addr_cells + 1]) << 32
            | p[i + addr_cells]);
        }

        printf("Adding memory range %16" PRIX64 " len %16" PRIX64 "\n",
            base_addr, base_size);

        gd_memory_map_entry ent = { 0 };
        ent.type       = gd_conventional_memory;
        ent.attributes = 0;
        ent.virtual_start = ent.physical_start = base_addr;
        ent.size = base_size;
        mmap_add_entry(ent);
    }

    dt_root = add_device_fdt(NULL, fdt, root, 0);
}
