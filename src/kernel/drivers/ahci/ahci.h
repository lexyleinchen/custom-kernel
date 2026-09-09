#ifndef AHCI_H
#define AHCI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ahci_controller_found(uint16_t bus, uint8_t slot, uint8_t function, uint8_t prog_if);

void ahci_init(void);

#ifdef __cplusplus
}
#endif

#endif // AHCI_H