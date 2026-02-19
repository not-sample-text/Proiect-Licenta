/**
 * @file input_events.cpp
 * @brief Input event queue implementation.
 */

#include "input_events.h"
#include "debug.h"

// ── Global Instance ─────────────────────────────────────────────
InputEventQueue g_input_queue;

// ── Implementation ──────────────────────────────────────────────
InputEventQueue::InputEventQueue()
    : m_head(0)
    , m_tail(0)
{}

bool InputEventQueue::enqueue(const InputEvent& event) {
    uint16_t next_head = (m_head + 1) % QUEUE_SIZE;
    
    // Check if queue is full
    if (next_head == m_tail) {
        // Queue full — drop event
        // Note: We don't log here because this might be called from ISR
        return false;
    }
    
    // Add event to buffer
    m_buffer[m_head] = event;
    
    // Advance head (atomic on single-word writes)
    m_head = next_head;
    
    return true;
}

bool InputEventQueue::dequeue(InputEvent& event) {
    // Check if queue is empty
    if (m_tail == m_head) {
        return false;
    }
    
    // Get event from buffer
    event = m_buffer[m_tail];
    
    // Advance tail
    m_tail = (m_tail + 1) % QUEUE_SIZE;
    
    return true;
}

bool InputEventQueue::is_empty() const {
    return m_head == m_tail;
}

uint16_t InputEventQueue::size() const {
    if (m_head >= m_tail) {
        return m_head - m_tail;
    } else {
        return QUEUE_SIZE - m_tail + m_head;
    }
}

void InputEventQueue::clear() {
    m_tail = m_head;
}
