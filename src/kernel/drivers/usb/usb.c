#include "usb.h"
#include "../pci/pci.h"
#include "../../inputs/mouse.h"
#include "../../core/log.h"

#define UHCI_FRAME_COUNT 1024
#define UHCI_LINK_TERMINATE 0x00000001
#define UHCI_LINK_QH 0x00000002
#define UHCI_PDI_SETUP 0x2D
#define UHCI_PDI_IN 0x69
#define UHCI_PDI_OUT 0xE1
#define UHCI_FIRST_DEVICE_ADDRESS 1
#define UHCI_ENDPOINT 0
#define UHCI_MAX_CONTROL_TDS 64
#define UHCI_LINK_DEPTH_FIRST 0x00000004
#define UHCI_TD_HOST_ERROR (1 << 16)
#define UHCI_TD_BITSTUFF_ERROR (1 << 17)
#define UHCI_TD_CRC_TIMEOUT (1 << 18)
#define UHCI_TD_NAK (1 << 19)
#define UHCI_TD_BABBLE (1 << 20)
#define UHCI_TD_BUFFER_ERROR (1 << 21)
#define UHCI_TD_STALLED (1 << 22)
#define UHCI_TD_ACTIVE (1 << 23)
#define UHCI_TD_IOC (1 << 24)
#define UHCI_TD_LOW_SPEED (1 << 26)
#define USB_DESCRIPTOR_CONFIGURATION 2
#define USB_DESCRIPTOR_INTERFACE 4
#define USB_DESCRIPTOR_ENDPOINT 5
#define USB_DESCRIPTOR_HID 0x21
#define USB_CLASS_HID 0x03
#define USB_HID_SUBCLASS_BOOT 0x01
#define USB_HID_PROTOCOL_MOUSE 0x02
#define USB_REQUEST_SET_PROTOCOL 0x0B
#define USB_ENDPOINT_IN 0x80
#define USB_TRANSFER_TYPE_INTERRUPT 0x03

static UsbDevice uhci_device;
static uint8_t uhci_current_device_address = 0;
static uint8_t uhci_next_device_address = 1;
static int uhci_low_speed = 0;
static uint16_t uhci_control_data_td_count = 0;
static int uhci_mouse_initialized = 0;

static uint32_t uhci_physical_address(const void* address) {
    return (uint32_t)(uintptr_t)address;
}

typedef struct {
    uint32_t link;
    uint32_t status;
    uint32_t token;
    uint32_t buffer;
    uint32_t software[4];
} __attribute__((packed, aligned(16))) UhciTd;

typedef struct {
    uint32_t head_link;
    uint32_t element_link;
    uint32_t software[2];
} __attribute__((packed, aligned(16))) UhciQh;

typedef struct {
    uint8_t request_type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} __attribute__((packed)) UsbSetupPacket;

static uint32_t uhci_frame_list[UHCI_FRAME_COUNT]
    __attribute__((aligned(4096)));

static UhciQh uhci_control_qh
    __attribute__((aligned(16)));

static UhciTd uhci_setup_td
    __attribute__((aligned(16)));

static UhciTd uhci_data_tds[UHCI_MAX_CONTROL_TDS]
    __attribute__((aligned(16)));

static UhciTd uhci_status_td
    __attribute__((aligned(16)));

static UsbSetupPacket uhci_setup_packet
    __attribute__((aligned(16)));

static uint8_t uhci_device_descriptor[18]
    __attribute__((aligned(16)));

static uint8_t uhci_configuration_descriptor[512]
    __attribute__((aligned(16)));

static uint8_t uhci_mouse_report[8]
    __attribute__((aligned(16)));

static UhciQh uhci_mouse_qh
    __attribute__((aligned(16)));

static UhciTd uhci_mouse_td
    __attribute__((aligned(16)));

static void uhci_setup_frame_list(void) {
    kernel_log("creating uhci frame list...");

    for (int i = 0; i < UHCI_FRAME_COUNT; i++) {
        uhci_frame_list[i] = UHCI_LINK_TERMINATE;
    }

    kernel_log("uhci frame list created.");
}

static void uhci_setup_control_qh(void) {
    kernel_log("creating uhci control queue...");
    uhci_control_qh.head_link = UHCI_LINK_TERMINATE;
    uhci_control_qh.element_link = uhci_physical_address(&uhci_setup_td);
    kernel_log("uhci control queue created.");
}

static void uhci_setup_mouse_qh(void) {
    uhci_mouse_qh.head_link = UHCI_LINK_TERMINATE;
    uhci_mouse_qh.element_link = uhci_physical_address(&uhci_mouse_td);
    kernel_log("ubs mouse queue created.");
}

static void uhci_create_setup_td(void) {
    uhci_setup_td.link = uhci_physical_address(&uhci_data_tds[0]) | UHCI_LINK_DEPTH_FIRST;
    uhci_setup_td.status = UHCI_TD_ACTIVE | (3 << 27) | (uhci_low_speed ? UHCI_TD_LOW_SPEED : 0);
    uhci_setup_td.token = (7 << 21) | (0 << 19) | (UHCI_ENDPOINT << 15) | (uhci_current_device_address << 8) | UHCI_PDI_SETUP;
    uhci_setup_td.buffer = uhci_physical_address(&uhci_setup_packet);
}

static int uhci_create_data_tds(void* buffer, uint16_t length) {
    uint16_t packet_size = uhci_device.max_packet_size;

    if (packet_size == 0) {
        kernel_log("usb control endpoint has invalid packet size.");
        return 0;
    }

    uint16_t td_count = (length + packet_size - 1) / packet_size;

    if (td_count > UHCI_MAX_CONTROL_TDS) {
        kernel_log("usb control transfer needs too many tds.");
        return 0;
    }

    uhci_control_data_td_count = td_count;
    uint8_t* data = (uint8_t*)buffer;

    for (uint16_t i = 0; i < td_count; i++) {
        uint16_t remaining = length - (i * packet_size);
        uint16_t transfer_size = remaining > packet_size ? packet_size : remaining;
        uhci_data_tds[i].link = (i + 1 < td_count) ? (uhci_physical_address(&uhci_data_tds[i + 1]) | UHCI_LINK_DEPTH_FIRST) : (uhci_physical_address(&uhci_status_td) | UHCI_LINK_DEPTH_FIRST);
        uhci_data_tds[i].status = UHCI_TD_ACTIVE | (3 << 27) | (uhci_low_speed ? UHCI_TD_LOW_SPEED : 0);
        uint32_t data_toggle = (i & 1) ? 0 : 1;
        uhci_data_tds[i].token = (((uint32_t)(transfer_size - 1)) << 21) | (data_toggle << 19) | (UHCI_ENDPOINT << 15) | (uhci_current_device_address << 8) | UHCI_PDI_IN;
        uhci_data_tds[i].buffer = uhci_physical_address(data);
        data += transfer_size;
    }

    return 1;
}

static void uhci_create_status_td(void) {
    uhci_status_td.link = UHCI_LINK_TERMINATE;
    uhci_status_td.status = UHCI_TD_ACTIVE | (3 << 27) | (uhci_low_speed ? UHCI_TD_LOW_SPEED : 0);
    uhci_status_td.token = (0x7FF << 21) | (1 << 19) | (UHCI_ENDPOINT << 15) | (uhci_current_device_address << 8) | UHCI_PDI_OUT;
    uhci_status_td.buffer = 0;
}

static void uhci_create_mouse_td(void) {
    uint16_t packet_size = uhci_device.interrupt_max_packet_size;

    if (packet_size > sizeof(uhci_mouse_report)) {
        packet_size = sizeof(uhci_mouse_report);
    }

    uhci_mouse_td.link = UHCI_LINK_TERMINATE;
    uhci_mouse_td.status = UHCI_TD_ACTIVE | (3 << 27) | (uhci_low_speed ? UHCI_TD_LOW_SPEED : 0);
    uhci_mouse_td.token = (((uint32_t)(packet_size - 1)) << 21) | (0 << 19) | ((uint32_t)uhci_device.interrupt_endpoint << 15) | ((uint32_t)uhci_current_device_address << 8) | UHCI_PDI_IN;
    uhci_mouse_td.buffer = uhci_physical_address(uhci_mouse_report);
}

static void uhci_schedule_mouse_qh(void) {
    uint32_t qh_address = uhci_physical_address(&uhci_mouse_qh);
    qh_address |= UHCI_LINK_QH;

    for (int i = 0; i < UHCI_FRAME_COUNT; i++) {
        uhci_frame_list[i] = qh_address;
    }

    kernel_log("usb mouse queue scheduled.");
}

static void uhci_mouse_process_report(void) {
    uint8_t buttons = uhci_mouse_report[0];
    int8_t dx = (int8_t)uhci_mouse_report[1];
    int8_t dy = (int8_t)uhci_mouse_report[2];
    kernel_log("mouse report buttons %u dx %d dy %d", (uint32_t)buttons, (int32_t)dx, (int32_t)dy);
    mouse_update(dx, dy, buttons);
}

static int uhci_td_has_error(const char* name, uint32_t status) {
    if (status & UHCI_TD_STALLED) {
        kernel_log("%s: usb transfer stalled.", name);
        return 1;
    }

    if (status & UHCI_TD_BUFFER_ERROR) {
        kernel_log("%s: usb buffer error.", name);
        return 1;
    }

    if (status & UHCI_TD_BABBLE) {
        kernel_log("%s: usb babble error.", name);
        return 1;
    }

    if (status & UHCI_TD_CRC_TIMEOUT) {
        kernel_log("%s: usb crc or timeout error.", name);
        return 1;
    }

    if (status & UHCI_TD_BITSTUFF_ERROR) {
        kernel_log("%s: usb bitstuff error.", name);
        return 1;
    }

    if (status & UHCI_TD_HOST_ERROR) {
        kernel_log("%s: usb host error.", name);
        return 1;
    }

    return 0;
}

static void uhci_mouse_poll(void) {
    uint32_t status = uhci_mouse_td.status;

    if (status & UHCI_TD_ACTIVE) {
        return;
    }

    if (uhci_td_has_error("mouse td", status)) {
        kernel_log("usb mouse transfer error.");
        uhci_create_mouse_td();
        return;
    }

    uhci_mouse_process_report();
    uhci_create_mouse_td();
}

static void uhci_schedule_control_qh(void) {
    uint32_t qh_address = uhci_physical_address(&uhci_control_qh);
    qh_address |= UHCI_LINK_QH;

    for (int i = 0; i < UHCI_FRAME_COUNT; i++) {
        uhci_frame_list[i] = qh_address;
    }

    kernel_log("uhci control queue scheduled.");
}

static void uhci_set_setup_packet(uint8_t request_type, uint8_t request, uint16_t value, uint16_t index, uint16_t length) {
    uhci_setup_packet.request_type = request_type;
    uhci_setup_packet.request = request;
    uhci_setup_packet.value = value;
    uhci_setup_packet.index = index;
    uhci_setup_packet.length = length;
}

static int uhci_wait_for_transfer(void) {
    kernel_log("waiting for usb transfer...");

    for (uint32_t timeout = 0; timeout < 5000000; timeout++) {
        if (uhci_setup_td.status & UHCI_TD_ACTIVE) {
            continue;
        }

        int data_active = 0;

        for (uint16_t i = 0; i < uhci_control_data_td_count; i++) {
            if (uhci_data_tds[i].status & UHCI_TD_ACTIVE) {
                data_active = 1;
                break;
            }
        }

        if (data_active) {
            continue;
        }

        if (uhci_status_td.status & UHCI_TD_ACTIVE) {
            continue;
        }

        kernel_log("usb transfer finished.");

        if (uhci_td_has_error("setup td", uhci_setup_td.status)) {
            return 0;
        }

        for (uint16_t i = 0; i < uhci_control_data_td_count; i++) {
            if (uhci_td_has_error("data td", uhci_data_tds[i].status)) {
                return 0;
            }
        }

        if (uhci_td_has_error("status td", uhci_status_td.status)) {
            return 0;
        }

        return 1;
    }

    kernel_log("usb transfer timed out.");
    return 0;
}

static int uhci_mouse_set_boot_protocol(void) {
    kernel_log("settings usb mouse boot protocol...");
    uhci_set_setup_packet(0x21, USB_REQUEST_SET_PROTOCOL, 0, uhci_device.interface_number, 0);
    uhci_setup_td.link = uhci_physical_address(&uhci_status_td) | UHCI_LINK_DEPTH_FIRST;
    uhci_setup_td.status = UHCI_TD_ACTIVE | (3 << 27) | (uhci_low_speed ? UHCI_TD_LOW_SPEED : 0);
    uhci_setup_td.token = (7 << 21) | (0 << 19) | (UHCI_ENDPOINT << 15) |(uhci_current_device_address << 8) | UHCI_PDI_SETUP;
    uhci_setup_td.buffer = uhci_physical_address(&uhci_setup_packet);
    uhci_status_td.link = UHCI_LINK_TERMINATE;
    uhci_status_td.status = UHCI_TD_ACTIVE | (3 << 27) | (uhci_low_speed ? UHCI_TD_LOW_SPEED : 0);
    uhci_status_td.token = (0x7FF << 21) | (1 << 19) | (UHCI_ENDPOINT << 15) |(uhci_current_device_address << 8) | UHCI_PDI_IN;
    uhci_status_td.buffer = 0;
    uhci_control_qh.element_link = uhci_physical_address(&uhci_setup_td);
    uhci_schedule_control_qh();

    if (!uhci_wait_for_transfer()) {
        kernel_log("failed to set usb mouse boot protocol.");
        return 0;
    }

    kernel_log("usb mouse boot protocol enabled.");
    return 1;
}

static int uhci_initialize_mouse(void) {
    if (uhci_device.interface_class != USB_CLASS_HID) {
        kernel_log("usb device is not hid.");
        return 0;
    }

    if (uhci_device.interface_subclass != USB_HID_SUBCLASS_BOOT) {
        kernel_log("usb hid device is not boot subclass.");
        return 0;
    }

    if (uhci_device.interface_protocol != USB_HID_PROTOCOL_MOUSE) {
        kernel_log("usb hid device is not a mouse.");
        return 0;
    }

    if (uhci_device.interrupt_endpoint == 0) {
        kernel_log("usb mouse has no interrupt endpoint.");
        return 0;
    }

    kernel_log("usb boot mouse detected.");

    if (!uhci_mouse_set_boot_protocol()) {
        return 0;
    }

    uhci_setup_mouse_qh();
    uhci_create_mouse_td();
    uhci_schedule_mouse_qh();
    uhci_mouse_initialized = 1;
    kernel_log("usb mouse initialized.");
    return 1;
}

static void outl(uint16_t port, uint32_t value) {
     __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

static void uhci_set_frame_list(uint16_t io_base) {
    uint32_t address = uhci_physical_address(uhci_frame_list);
    kernel_log("uhci frame list address %u", address);
    kernel_log("uhci frame list alignment %u", address & 0xFFF);
    outl(io_base + 0x08, address);
}

static void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void uhci_start(uint16_t io_base) {
    uint16_t command = inw(io_base + 0x00);
    command |= 0x001;
    command |= 0x080;
    outw(io_base + 0x00, command);
    kernel_log("uhci controller started.");
}

static void uhci_create_get_descriptor(uint8_t descriptor_type, uint8_t descriptor_index, uint16_t length) {
    uhci_set_setup_packet(0x80, 0x06, ((uint16_t)descriptor_type << 8) | descriptor_index, 0, length);
    kernel_log("created get descriptor request type %u length %u", (uint32_t)descriptor_type, (uint32_t)length);
}

static void uhci_create_get_descriptor_transfer(uint8_t descriptor_type, uint8_t descriptor_index, void* buffer, uint16_t length) {
    kernel_log("creating usb get descriptor transfer...");
    uhci_create_get_descriptor(descriptor_type, descriptor_index, length);
    uhci_create_setup_td();

    if (!uhci_create_data_tds(buffer, length)) {
        return;
    }

    uhci_create_status_td();
    uhci_control_qh.element_link = uhci_physical_address(&uhci_setup_td);;
    kernel_log("usb get descriptor transfer created.");
}

static void uhci_decode_device_descriptor(UsbDevice* device) {
    device->max_packet_size = uhci_device_descriptor[7];
    device->vendor_id = (uint16_t)uhci_device_descriptor[8] | ((uint16_t)uhci_device_descriptor[9] << 8);
    device->product_id = (uint16_t)uhci_device_descriptor[10] | ((uint16_t)uhci_device_descriptor[11] << 8);
    device->device_class = uhci_device_descriptor[4];
    device->device_subclass = uhci_device_descriptor[5];
    device->device_protocol = uhci_device_descriptor[6];
    kernel_log("usb vendor_id %u", (uint32_t)device->vendor_id);
    kernel_log("usb product_id %u", (uint32_t)device->product_id);
    kernel_log("usb max_packet_size %u", (uint32_t)device->max_packet_size);
    kernel_log("usb device_class %u", (uint32_t)device->device_class);
    kernel_log("usb device_subclass %u", (uint32_t)device->device_subclass);
    kernel_log("usb device_protocol %u", (uint32_t)device->device_protocol);
    kernel_log("usb device descriptor class %u", (uint32_t)uhci_device_descriptor[4]);
    kernel_log("usb device descriptor subclass %u", (uint32_t)uhci_device_descriptor[5]);
    kernel_log("usb device descriptor protocol %u", (uint32_t)uhci_device_descriptor[6]);
}

static void uhci_print_device_descriptor(void) {
    kernel_log("usb device descriptor recived.");

    for (int i = 0; i < 18; i++) {
        kernel_log("descriptor byte %u = %u", (uint32_t)i, (uint32_t)uhci_device_descriptor[i]);
    }
}

static void uhci_debug_controller(uint16_t io_base) {
    uint16_t command = inw(io_base + 0x00);
    uint16_t status = inw(io_base + 0x02);
    uint16_t frame = inw(io_base + 0x06);
    uint32_t frame_list = uhci_physical_address(uhci_frame_list);
    kernel_log("uhci command = %u", (uint32_t)command);
    kernel_log("uhci status = %u", (uint32_t)status);
    kernel_log("uhci frame = %u", (uint32_t)frame);
    kernel_log("uhci frame_list = %u", frame_list);
    kernel_log("uhci frame 0 = %u", uhci_frame_list[0]);
    kernel_log("uhci qh = %u", uhci_physical_address(&uhci_control_qh));
    kernel_log("uhci setup_td = %u", uhci_physical_address(&uhci_setup_td));
    kernel_log("uhci data_td[0] = %u", uhci_physical_address(&uhci_data_tds[0]));
    kernel_log("uhci status_td = %u", uhci_physical_address(&uhci_status_td));
}

static uint8_t usb_allocate_device_address(void) {
    if (uhci_next_device_address > 127) {
        kernel_log("no usb device address available.");
        return 0;
    }

    uint8_t address = uhci_next_device_address;
    uhci_next_device_address++;
    kernel_log("allocated usb device address %u", (uint32_t)address);
    return address;
}

static int uhci_set_device_address(uint8_t address) {
    kernel_log("setting usb device address to %u", (uint32_t)address);
    uhci_set_setup_packet(0x00, 0x05, address, 0, 0);
    uhci_setup_td.link = uhci_physical_address(&uhci_status_td);
    uhci_setup_td.status = UHCI_TD_ACTIVE | (3 << 27) | (uhci_low_speed ? UHCI_TD_LOW_SPEED : 0);
    uhci_setup_td.token = (7 << 21) | (0 << 19) | (UHCI_ENDPOINT << 15) | (0 << 8) | UHCI_PDI_SETUP;
    uhci_setup_td.buffer = uhci_physical_address(&uhci_setup_packet);
    uhci_status_td.link = UHCI_LINK_TERMINATE;
    uhci_status_td.status = UHCI_TD_ACTIVE | (3 << 27) | (uhci_low_speed ? UHCI_TD_LOW_SPEED : 0);
    uhci_status_td.token = (0x7FF << 21) | (1 << 19) | (UHCI_ENDPOINT << 15) | (uhci_current_device_address << 8) | UHCI_PDI_IN;
    uhci_status_td.buffer = 0;
    uhci_control_qh.element_link = uhci_physical_address(&uhci_setup_td);
    uhci_schedule_control_qh();

    if (!uhci_wait_for_transfer()) {
        kernel_log("failed to set usb device address.");
        return 0;
    }

    uhci_current_device_address = address;
    kernel_log("usb device address set.");
    return 1;
}

static int uhci_get_configuration_descriptor_header(void) {
    kernel_log("requesting usb configuration descriptor header...");
    uhci_create_get_descriptor_transfer(0x02, 0, uhci_configuration_descriptor, 9);
    uhci_schedule_control_qh();

    if (!uhci_wait_for_transfer()) {
        kernel_log("failed to get configuration descriptor header.");
        return 0;
    }

    kernel_log("usb configuration descriptor header recived.");

    for (int i = 0; i < 9; i++) {
        kernel_log("config descriptor byte %u = %u", (uint32_t)i, (uint32_t)uhci_configuration_descriptor[i]);
    }

    return 1;
}

static uint16_t uhci_get_configuration_total_length(void) {
    uint16_t length = (uint16_t)uhci_configuration_descriptor[2] | ((uint16_t)uhci_configuration_descriptor[3] << 8);
    kernel_log("usb configuration total length %u", (uint32_t)length);
    return length;
}

static void uhci_print_configuration_descriptor(uint16_t total_length) {
    kernel_log("usb configuration descriptor:");

    for (uint16_t i = 0; i < total_length; i++) {
        kernel_log("config byte %u = %u", (uint32_t)i, (uint32_t)uhci_configuration_descriptor[i]);
    }
}

static int uhci_get_configuration_descriptor(uint16_t total_length) {
    if (total_length < 9)  {
        kernel_log("invalid usb configuration descriptor length.");
        return 0;
    }

    if (total_length > sizeof(uhci_configuration_descriptor)) {
        kernel_log("usb configuration is too large.");
        return 0;
    }

    kernel_log("requesting full usb configuration descriptor length %u", (uint32_t)total_length);
    uhci_create_get_descriptor_transfer(0x02, 0, uhci_configuration_descriptor, total_length);
    uhci_schedule_control_qh();

    if (!uhci_wait_for_transfer()) {
        kernel_log("failed to get full configuration descriptor.");
        return 0;
    }

    kernel_log("full usb configuration descriptor recived.");
    uhci_print_configuration_descriptor(total_length);
    return 1;
}

static int uhci_parse_configuration_descriptor(uint16_t total_length) {
    kernel_log("parsing usb configuration descriptor...");
    uint16_t offset = 0;
    int hid_mouse_interface = 0;

    while (offset < total_length) {
        uint8_t length = uhci_configuration_descriptor[offset];
        uint8_t type = uhci_configuration_descriptor[offset + 1];

        if (length < 2) {
            kernel_log("invalid usb descriptor length.");
            return 0;
        }

        if ((uint32_t)offset + length > total_length) {
            kernel_log("usb descriptor extends past configuration.");
            return 0;
        }

        kernel_log("usb descriptor offset %u length %u type %u", (uint32_t)offset, (uint32_t)length, (uint32_t)type);

        if (type == USB_DESCRIPTOR_CONFIGURATION) {
            if (length < 9) {
                kernel_log("invalid configuration descriptor.");
                return 0;
            }

            uhci_device.configuration_value = uhci_configuration_descriptor[offset + 5];
            kernel_log("usb configuration value %u", (uint32_t)uhci_device.configuration_value);
        }
        else if (type == USB_DESCRIPTOR_INTERFACE) {
            if (length < 9) {
                kernel_log("invalid interface descriptor.");
                return 0;
            }

            uhci_device.interface_number = uhci_configuration_descriptor[offset + 2];
            uhci_device.interface_class = uhci_configuration_descriptor[offset + 5];
            uhci_device.interface_subclass = uhci_configuration_descriptor[offset + 6];
            uhci_device.interface_protocol = uhci_configuration_descriptor[offset + 7];
            kernel_log("usb interface %u", (uint32_t)uhci_device.interface_number);
            kernel_log("usb class %u", (uint32_t)uhci_device.interface_class);
            kernel_log("usb subclass %u", (uint32_t)uhci_device.interface_subclass);
            kernel_log("usb protocol %u", (uint32_t)uhci_device.interface_protocol);

            if (uhci_device.interface_class == USB_CLASS_HID && uhci_device.interface_subclass == USB_HID_SUBCLASS_BOOT && uhci_device.interface_protocol == USB_HID_PROTOCOL_MOUSE) {
                hid_mouse_interface = 1;
                kernel_log("usb hid boot mouse interface found.");
            }
        }
        else if(type == USB_DESCRIPTOR_ENDPOINT) {
            if (length < 7) {
                kernel_log("invalid endpoint descriptor");
                return 0;
            }

            uint8_t endpoint_address = uhci_configuration_descriptor[offset + 2];
            uint8_t attributes = uhci_configuration_descriptor[offset + 3];
            uint16_t max_packet_size = (uint16_t)uhci_configuration_descriptor[offset + 4] | ((uint16_t)uhci_configuration_descriptor[offset + 5] << 8);
            uint8_t interval = uhci_configuration_descriptor[offset + 6];
            kernel_log("usb endpoint address %u", (uint32_t)endpoint_address);
            kernel_log("usb endpoint attributes %u", (uint32_t)attributes);
            kernel_log("usb endpoint max packet size %u", (uint32_t)max_packet_size);
            kernel_log("usb endpoint interval %u", (uint32_t)interval);

            if (hid_mouse_interface && (endpoint_address & USB_ENDPOINT_IN) && ((attributes & 0x03) == USB_TRANSFER_TYPE_INTERRUPT)) {
                uhci_device.interrupt_endpoint = endpoint_address & 0x0F;
                uhci_device.interrupt_max_packet_size = max_packet_size;
                uhci_device.interrupt_interval = interval;
                kernel_log("usb mouse interrupt in endpoint found.");
            }
        }

        offset += length;
    }

    if (!hid_mouse_interface) {
        kernel_log("usb hid boot mouse interface not found.");
    }

    if (uhci_device.interrupt_endpoint == 0) {
        kernel_log("usb interrupt in endpoint not found.");
    }

    return 1;
}

static int uhci_set_configuration(void) {
    kernel_log("setting usb configuration to %u", (uint32_t)uhci_device.configuration_value);
    uhci_set_setup_packet(0x00, 0x09, uhci_device.configuration_value, 0, 0);
    uhci_setup_td.link = uhci_physical_address(&uhci_status_td);
    uhci_setup_td.status = UHCI_TD_ACTIVE | (3 << 27) | (uhci_low_speed ? UHCI_TD_LOW_SPEED : 0);
    uhci_setup_td.token = (7 << 21) | (0 << 19) | (UHCI_ENDPOINT << 15) | (uhci_current_device_address << 8) | UHCI_PDI_SETUP;
    uhci_setup_td.buffer = uhci_physical_address(&uhci_setup_packet);
    uhci_status_td.link = UHCI_LINK_TERMINATE;
    uhci_status_td.status = UHCI_TD_ACTIVE | (3 << 27) | (uhci_low_speed ? UHCI_TD_LOW_SPEED : 0);
    uhci_status_td.token = (0x7FF << 21) | (1 << 19) | (UHCI_ENDPOINT << 15) | (uhci_current_device_address << 8) | UHCI_PDI_IN;
    uhci_status_td.buffer = 0;
    uhci_control_qh.element_link = uhci_physical_address(&uhci_setup_td);
    uhci_schedule_control_qh();

    if (!uhci_wait_for_transfer()) {
        kernel_log("failed to set usb configuration.");
        return 0;
    }

    kernel_log("usb configuration set.");
    return 1;
}

static int uhci_get_device_descriptor(uint16_t io_base) {
    kernel_log("requesting usb device descriptor...");
    uhci_device.max_packet_size = 8;
    uhci_create_get_descriptor_transfer(0x01, 0, uhci_device_descriptor, 18);
    uhci_schedule_control_qh();
    kernel_log("frame 0 value = %u", uhci_frame_list[0]);
    kernel_log("frame 137 value = %u", uhci_frame_list[137]);
    kernel_log("qh_head = %u", uhci_control_qh.head_link);
    kernel_log("qh_element = %u", uhci_control_qh.element_link);
    kernel_log("setup_td_link = %u", uhci_setup_td.link);
    kernel_log("data_td_link = %u", uhci_data_tds[0].link);
    kernel_log("status_td_link = %u", uhci_status_td.link);
    kernel_log("uhci state after scheduling:");
    uhci_debug_controller(io_base);

    if (!uhci_wait_for_transfer()) {
        kernel_log("failed to get usb device descriptor.");
        return 0;
    }

    uhci_print_device_descriptor();
    uhci_decode_device_descriptor(&uhci_device);

    uint8_t address = usb_allocate_device_address();

    if (address == 0) {
        return 0;
    }

    if (!uhci_set_device_address(address)) {
        kernel_log("failed to assign usb device address.");
        return 0;
    }

    uhci_device.address = address;

    if (!uhci_get_configuration_descriptor_header()) {
        kernel_log("failed to get usb configuration header.");
        return 0;
    }

    uint16_t total_length = uhci_get_configuration_total_length();

    if (!uhci_get_configuration_descriptor(total_length)) {
        kernel_log("failed to get usb configuration descriptor.");
        return 0;
    }

    if (!uhci_parse_configuration_descriptor(total_length)) {
        kernel_log("failed to parse usb configuration descriptor.");
        return 0;
    }

    if (!uhci_set_configuration()) {
        kernel_log("failed to set usb configuration.");
        return 0;
    }

    if (!uhci_initialize_mouse()) {
        kernel_log("usb mouse initialization failed.");
        return 0;
    }

    return 1;
}

void usb_poll(void) {
    if (!uhci_mouse_initialized) {
        return;
    }

    uhci_mouse_poll();
}

static void uhci_reset(uint16_t io_base) {
    kernel_log("resettting uhci controller...");
    outw(io_base + 0x00, 0x0002);
    
    for (volatile int i = 0; i < 100000; i++) {
        uint16_t command = inw(io_base + 0x00);

        if ((command & 0x0002) == 0) {
            break;
        }
    }

    outw(io_base + 0x00, 0x0000);
    kernel_log("uhci controller reseted.");
}

static int uhci_reset_port(uint16_t port) {
    kernel_log("reseting usb port...");
    uint16_t status = inw(port);
    status |= 0x0200;
    outw(port, status);

    for (volatile int i = 0; i < 1000000; i++) {
        __asm__ volatile ("pause");
    }

    status = inw(port);
    status &= ~0x0200;
    outw(port, status);

    for (volatile int i = 0; i < 500000; i++) {
        __asm__ volatile ("pause");
    }

    status = inw(port);
    status |= 0x0004;
    outw(port, status);

    for (volatile int i = 0; i < 100000; i++) {
        __asm__ volatile ("pause");
    }

    status = inw(port);
    kernel_log("usb port status after reset %u", (uint32_t)status);
    uhci_low_speed = (status & 0x0100) != 0;

    if (uhci_low_speed) {
        kernel_log("usb device is low speed.");
    }
    else{
        kernel_log("usb device is full speed.");
    }

    if ((status & 0x0004) == 0) {
        kernel_log("usb port failed to enable.");
        return 0;
    }

    kernel_log("usb port enabled.");
    return 1;
}

static void uhci_check_ports(uint16_t io_base, uint32_t io_size) {
    kernel_log("checking uhci ports...");

    if (io_size <= 0x10) {
        kernel_log("uhci: no port registers found.");
        return;
    }

    uint32_t port_register_size = io_size - 0x10;
    int maximum_ports = port_register_size / 2;
    int port_count = 0;

    for (int i = 0; i < maximum_ports; i++) {
        uint16_t port_address = io_base + 0x10 + (i * 2);
        uint16_t status = inw(port_address);

        if (status == 0xFFFF) {
            break;
        }

        if ((status & 0x0080) == 0) {
            break;
        }

        port_count++;
    }

    kernel_log("uhci controller has %u ports", (uint32_t)port_count);

    for (int i = 0; i < port_count; i++) {
        uint16_t port_address = io_base + 0x10 + (i * 2);
        uint16_t status = inw(port_address);
        kernel_log("uhci port %u status %u", (uint32_t)(i + 1), (uint32_t)status);

        if (status & 0x0001) {
            kernel_log("uhci port %u: usb device connected." , (uint32_t)(i + 1));

            if(uhci_reset_port(port_address)) {
                kernel_log("uhci port %u: ready for usb transfers.", (uint32_t)(i + 1));
                uhci_get_device_descriptor(io_base);
            }
            else {
                kernel_log("uhci port %u: failed to prepare.", (uint32_t)(i + 1));
            }
        }
        else {
            kernel_log("uhci port %u: no usb device connected.", (uint32_t)(i + 1));
        }
    }
}

void usb_controller_found(uint16_t bus, uint8_t slot, uint8_t function, uint8_t prog_if) {
    kernel_log("found usb controller = bus: %u | slot: %u | function: %u | prog_if %u", bus, slot, function, prog_if);
    uint32_t bar;

    if (prog_if == 0x00) {
        bar = pci_config_read32(bus, slot, function, 0x20);
        bar &= 0xFFFFFFFC;
        uint32_t bar_size = pci_get_bar_size(bus, slot, function, 0x20);
        kernel_log("uhci io base %u", bar);
        kernel_log("uhci io size %u", bar_size);
        uint32_t command = pci_config_read32(bus, slot, function, 0x04);
        command |= 0x00000005;
        pci_config_write32(bus, slot, function, 0x04, command);
        kernel_log("uhci pci controller enabled.");
        uhci_reset((uint16_t)bar);
        uhci_setup_frame_list();
        uhci_setup_control_qh();
        uhci_set_frame_list((uint16_t)bar);
        uhci_start((uint16_t)bar);
        uhci_debug_controller((uint16_t)bar);
        uhci_check_ports((uint16_t)bar, bar_size);
    }
    else if (prog_if == 0x20) {
        bar = pci_config_read32(bus, slot, function, 0x10);
        bar &= 0xFFFFFFF0;
        kernel_log("ehci memory base %u", bar);
        uint32_t command = pci_config_read32(bus, slot, function, 0x04);
        command |= 0x00000006;
        pci_config_write32(bus, slot, function, 0x04, command);
        kernel_log("ehci pci controller enabled.");
    }
}