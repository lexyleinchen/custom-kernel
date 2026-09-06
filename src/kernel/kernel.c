#include <stdint.h>

#include "log.h"
#include "multiboot.h"
#include "framebuffer.h"
#include "pci.h"
#include "usb.h"
#include "ps2.h"
#include "../os/os.h"

void kernel_main(uint32_t multiboot_address) {
    log_init();
    kernel_log("PrintOS Kernel Starting...");
    multiboot_init(multiboot_address);
    pci_init();
    os_init();
    ps2_init();
    kernel_log("kernel started.");

    while (1) {
        ps2_poll();
        usb_poll();
        os_draw();
    }
}