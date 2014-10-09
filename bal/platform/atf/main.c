#include <gd_bal.h>
#include <bal/device/uart/arm_pl011.h>
#include <stdio.h>
#include <inttypes.h>
#include <_PDCLIB_io.h>
#include <libfdt.h>

DEFINE_ARM_PL011(uart0, { (void*) 0x1C090000 }, 24000000);

static void walk_children(void *fdt, int depth, int offset)
{
    printf("%*c -> %s is ", depth, ' ', fdt_get_name(fdt, offset, NULL));

    int len;
    const char* compatible = fdt_getprop(fdt, offset, "compatible", &len);
    for (int p = 0; p < len; p += strlen(compatible + p) + 1) {
        printf("\"%s\" ", compatible + p);
    }
    printf("\n");

    int suboff = fdt_first_subnode(fdt, offset);
    while (suboff != -FDT_ERR_NOTFOUND) {
        walk_children(fdt, depth+1, suboff);
        suboff = fdt_next_subnode(fdt, suboff);
    }
}

void bal_main_atf(void *pdtree);
void bal_main_atf(void *pdtree)
{
    arm_pl011_init(&uart0);
    stdin->handle = stdout->handle = stderr->handle = ((_PDCLIB_fd_t) {&uart0} );

    printf("FDT at %p\n", pdtree);

    int rv = fdt_check_header(pdtree);
    if (rv) {
        printf("Error: %s\n", fdt_strerror(rv));
    }

    for (int i = 0; i < fdt_num_mem_rsv(pdtree); i++) {
        uint64_t addr, sz;
        fdt_get_mem_rsv(pdtree, i, &addr, &sz);
        printf("Reserved mem: %" PRIX64 " len=%" PRIX64 "\n", addr, sz);
    }

    printf("Tree:\n");
    walk_children(pdtree, 1, fdt_next_node(pdtree, -1, NULL));

    for(;;);
}
