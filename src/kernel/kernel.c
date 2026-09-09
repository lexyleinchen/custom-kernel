#include <stdint.h>

#include "core/log.h"
#include "boot/multiboot.h"
#include "framebuffer/framebuffer.h"
#include "drivers/pci/pci.h"
#include "drivers/usb/usb.h"
#include "drivers/ide/ide.h"
#include "drivers/ps2/ps2.h"
#include "storage/storage.h"
#include "../os/os.h"

void kernel_main(uint32_t multiboot_address) {
    log_init();
    kernel_log("PrintOS Kernel Starting...");
    multiboot_init(multiboot_address);
    pci_init();
    ide_init();
    storage_init();
    os_init();
    ps2_init();
    kernel_log("kernel started.");

    while (1) {
        ps2_poll();
        usb_poll();
        os_draw();
    }
}