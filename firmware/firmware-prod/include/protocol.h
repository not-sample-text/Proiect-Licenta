/**
 * @file protocol.h
 * @brief Compact serial protocol for the PROS3 Macropad.
 *
 * Encodes a key/encoder event into a single byte, transmitted as
 * 2 hex characters + '\n'  (3 bytes on the wire).
 *
 * Bit layout of the payload byte:
 *
 *   Bit  7  6  │ 5  4 │ 3  2 │ 1  0
 *       Layer  │ Col  │ Row  │ Event
 *       (0-3)  │(0-2) │(0-3) │ type
 *
 * Event types:
 *   0b00 = KEY_PRESS
 *   0b01 = KEY_RELEASE
 *   0b10 = ENCODER_CW   (clockwise)
 *   0b11 = ENCODER_CCW   (counter-clockwise)
 *
 * Example: Layer 2, Col 1, Row 3, press
 *   → 0b10_01_11_00 → 0x9C → "9C\n"
 */

#pragma once

#include <Arduino.h>

// ── Event types ─────────────────────────────────────────────────
enum EventType : uint8_t {
    EVT_KEY_PRESS   = 0b00,
    EVT_KEY_RELEASE = 0b01,
    EVT_ENC_CW      = 0b10,
    EVT_ENC_CCW     = 0b11,
};

// ── Encode ──────────────────────────────────────────────────────
/**
 * Pack a macropad event into a single byte.
 *
 * @param layer  Active layer   (0-3)
 * @param col    Matrix column  (0-2, ignored for encoder events)
 * @param row    Matrix row     (0-3, ignored for encoder events)
 * @param evt    Event type
 * @return       Packed byte
 */
inline uint8_t protocol_encode(uint8_t layer, uint8_t col, uint8_t row, EventType evt) {
    return ((layer & 0x03) << 6)
         | ((col   & 0x03) << 4)
         | ((row   & 0x03) << 2)
         | ((uint8_t)evt   & 0x03);
}

// ── Decode ──────────────────────────────────────────────────────
inline uint8_t   protocol_layer(uint8_t b) { return (b >> 6) & 0x03; }
inline uint8_t   protocol_col  (uint8_t b) { return (b >> 4) & 0x03; }
inline uint8_t   protocol_row  (uint8_t b) { return (b >> 2) & 0x03; }
inline EventType protocol_event(uint8_t b) { return (EventType)(b & 0x03); }

// ── Send ────────────────────────────────────────────────────────
/**
 * Encode and send an event over a Stream (Serial, BLE UART, etc.).
 * Format: 2 uppercase hex chars + '\n'  (e.g. "9C\n")
 */
inline void protocol_send(Stream &out, uint8_t layer, uint8_t col,
                           uint8_t row, EventType evt) {
    uint8_t b = protocol_encode(layer, col, row, evt);
    char buf[4];
    snprintf(buf, sizeof(buf), "%02X\n", b);
    out.print(buf);
}
