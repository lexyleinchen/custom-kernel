#include "ahci.h"
#include "../pci/pci.h"
#include "../../storage/block.h"
#include "../../core/log.h"

#define PCI_CLASS_MASS_STORAGE 0x01
#define PCI_SUBCLASS_SATA 0x06
#define AHCI_PROG_IF 0x01
#define PCI_BAR5 0x24
#define AHCI_CAP 0x00
#define AHCI_GHC 0x04
#define AHCI_PI 0x0C
#define AHCI_PORT_BASE 0x100
#define AHCI_PORT_CLB 0x00
#define AHCI_PORT_CLBU 0x04
#define AHCI_PORT_FB 0x08
#define AHCI_PORT_FBU 0x0C
#define AHCI_PORT_IS 0x10
#define AHCI_PORT_CMD 0x18
#define AHCI_PORT_TFD 0x20
#define AHCI_PORT_SIG 0x24
#define AHCI_PORT_SSTS 0x28
#define AHCI_PORT_SCTL 0x2C
#define AHCI_PORT_SERR 0x30
#define AHCI_PORT_CMD_ST (1 << 0)
#define AHCI_PORT_CMD_FRE (1 << 4)
#define AHCI_PORT_CMD_FR (1 << 14)
#define AHCI_PORT_CMD_CR (1 << 15)
#define AHCI_PORT_TFD_BSY (1 << 7)
#define AHCI_PORT_TFD_DRQ (1 << 3)
#define AHCI_SSTS_DET_MASK 0x0F
#define AHCI_SSTS_DET_PRESENT 0x03
#define AHCI_SIG_ATA 0x00000101
#define AHCI_SIG_ATAPI 0xEB140101
#define AHCI_SIG_SEMB 0xC33C0101
#define AHCI_SIG_PM 0x96690101

static uint32_t ahci_base = 0;

void ahci_controller_found(uint16_t bus, uint8_t slot, uint8_t function, uint8_t prog_if) {
    kernel_log("found ahci controller = bus: %u | slot: %u | function: %u | prog_if %u", bus, slot, function, prog_if);
    uint32_t bar5 = pci_config_read32(bus, slot, function, PCI_BAR5);
    kernel_log("ahci bar5 = %u", bar5);

    if (bar5 & 0x01) {
        kernel_log("ahci bar5 is i/o space.");
        return;
    }

    ahci_base = bar5 & 0xFFFFFFF0;
    kernel_log("ahci mmio base = %u", ahci_base);
}