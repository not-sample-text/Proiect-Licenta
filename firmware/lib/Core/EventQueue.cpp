#include "EventQueue.h"

InputEvent EventQueue::buffer[CAPACITY];
volatile uint8_t EventQueue::head = 0;
volatile uint8_t EventQueue::tail = 0;

bool EventQueue::enqueue(InputEvent event) {
    uint8_t nextHead = (head + 1) % CAPACITY;
    
    // Check if buffer is full
    if (nextHead == tail) {
        return false;
    }
    
    buffer[head] = event;
    head = nextHead;
    return true;
}

bool EventQueue::dequeue(InputEvent& event) {
    // Check if buffer is empty
    if (head == tail) {
        return false;
    }
    
    event = buffer[tail];
    tail = (tail + 1) % CAPACITY;
    return true;
}

bool EventQueue::isEmpty() {
    return head == tail;
}
