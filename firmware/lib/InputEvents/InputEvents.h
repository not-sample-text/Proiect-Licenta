/**
 * @file input_events.h
 * @brief Event queue implementation for macropad input handling.
 *
 * This file defines a lightweight, lock-free ring buffer for storing input
 * events such as key presses/releases and encoder rotations. The event queue
 * is designed to be ISR-safe, allowing interrupt handlers to enqueue events
 * without blocking the main application loop.
 *
 * Key Features:
 * - FIFO (First-In-First-Out) queue for predictable event processing.
 * - ISR-safe enqueue operation for real-time input handling.
 * - Simple dequeue operation for the main application loop.
 * - Support for multiple event types, including key and encoder events.
 */

#pragma once

#include <Arduino.h>

// ── Event Types ─────────────────────────────────────────────────
enum InputEventType : uint8_t {
    EVENT_KEY_PRESS = 0,
    EVENT_KEY_RELEASE,
    EVENT_ENCODER_CW,       // Clockwise rotation
    EVENT_ENCODER_CCW,      // Counter-clockwise rotation
    EVENT_ENCODER_BUTTON_PRESS,
    EVENT_ENCODER_BUTTON_RELEASE,
};

// ── Event Structure ─────────────────────────────────────────────
struct InputEvent {
    InputEventType type;
    uint8_t        col;        // Matrix column (0-2), unused for encoder
    uint8_t        row;        // Matrix row (0-3), unused for encoder
    uint32_t       timestamp;  // millis() when event was generated
    
    InputEvent()
        : type(EVENT_KEY_PRESS)
        , col(0)
        , row(0)
        , timestamp(0)
    {}
    
    InputEvent(InputEventType t, uint8_t c, uint8_t r, uint32_t ts = 0)
        : type(t)
        , col(c)
        , row(r)
        , timestamp(ts ? ts : millis())
    {}
};

// ── Event Queue ─────────────────────────────────────────────────
/**
 * Lock-free ring buffer for input events.
 * 
 * Thread-safety:
 * - Enqueue is ISR-safe (single producer)
 * - Dequeue is main-loop only (single consumer)
 * - No locking required due to single-producer-single-consumer design
 */
class InputEventQueue {
public:
    InputEventQueue();
    
    /**
     * Add an event to the queue (ISR-safe).
     * @return true if event was added, false if queue is full
     */
    bool enqueue(const InputEvent& event);
    
    /**
     * Remove and return the next event from the queue.
     * @param[out] event The dequeued event
     * @return true if an event was dequeued, false if queue is empty
     */
    bool dequeue(InputEvent& event);
    
    /**
     * Check if the queue is empty.
     */
    bool is_empty() const;
    
    /**
     * Get the number of events currently in the queue.
     */
    uint16_t size() const;
    
    /**
     * Clear all events from the queue.
     */
    void clear();
    
    /**
     * Get the maximum queue capacity.
     */
    static constexpr uint16_t capacity() { return QUEUE_SIZE; }

private:
    static constexpr uint16_t QUEUE_SIZE = 32;  // Power of 2 for fast wrap-around
    
    InputEvent   m_buffer[QUEUE_SIZE];
    volatile uint16_t m_head;  // Write index (producer)
    volatile uint16_t m_tail;  // Read index (consumer)
};

// ── Global Event Queue ──────────────────────────────────────────
extern InputEventQueue g_input_queue;
