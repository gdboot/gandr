#include <gd_bal.h>
#include <bal/device/uart/arm_pl011.h>

DEFINE_ARM_PL011(uart0, { (void*) 0x1C090000 }, 24000000);

void bal_main_atf(void *pdtree);
void bal_main_atf(void *pdtree)
{
    arm_pl011_init(&uart0);
    size_t written;
    const char *hello = "Hello, world!\r\n";
    gd_write(&uart0.dev, hello, strlen(hello), &written);
    for(;;);
}
