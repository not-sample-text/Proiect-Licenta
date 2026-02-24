#ifndef USB_HID_H
#define USB_HID_H

#include <stdint.h>

/**
 * @file usb_hid.h
 * @brief USB HID interface for the PROS3 Macropad firmware.
 *
 * This file declares the functions and data structures required for implementing
 * USB HID functionality using the TinyUSB stack. It supports both keyboard and
 * consumer control reports, enabling the macropad to act as a composite HID device.
 *
 * Key Responsibilities:
 * - Sending keyboard reports for key presses and modifier combinations.
 * - Sending consumer control reports for media keys and other controls.
 * - Managing the TinyUSB HID task and handling USB events.
 * - Providing an interface for other modules to send HID reports.
 */

// Function declarations
void send_hid_report(uint8_t keycode);
void send_consumer_report(uint16_t usage_code);
void hid_task(void);
void setup_usb_hid(void);
void usb_hid_task(void);

#endif // USB_HID_H
