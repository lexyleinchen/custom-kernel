#ifndef USB_H
#define USB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UsbDevice {
    uint8_t address;
    uint8_t port;
    uint16_t max_packet_size;
    uint16_t vendor_id;
    uint16_t product_id;
    uint8_t device_class;
    uint8_t device_subclass;
    uint8_t device_protocol;
    uint8_t configuration_value;
    uint8_t interface_number;
    uint8_t interface_class;
    uint8_t interface_subclass;
    uint8_t interface_protocol;
    uint8_t interrupt_endpoint;
    uint16_t interrupt_max_packet_size;
    uint8_t interrupt_interval;
} UsbDevice;

void usb_controller_found(uint16_t bus, uint8_t slot, uint8_t function, uint8_t prog_if);

void usb_poll(void);

#ifdef __cplusplus
}
#endif

#endif // USB_H