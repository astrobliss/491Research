#include "src/pch.h"
#include "SPSCFastForwardQueue.h"
#include "src/Utilities/QueueUtilities.cpp"

SPSCFastForwardQueue::SPSCFastForwardQueue(int maxCapacity) {
    head = tail = 0;
    capacity = maxCapacity;
    buffer = new UINT64[capacity];
    for (int i = 0; i < capacity; ++i) {
        buffer[i] = EMPTY_SLOT;
    }
}

SPSCFastForwardQueue::~SPSCFastForwardQueue() {
    delete[] buffer;
    delete[] oversizedCacheLine;
}

void SPSCFastForwardQueue::push(UINT64 newElement) {
    int nextSlot = nextBufferElement(tail, capacity);
    while (buffer[tail] != EMPTY_SLOT) {
        // Queue is full, wait for an element to be popped
        Sleep(PRODUCER_TIMEOUT_MS);
    }
    buffer[tail] = newElement;
    tail = nextSlot;
}

UINT64 SPSCFastForwardQueue::pop() {
    UINT64 currentElement;
    while ( (currentElement = buffer[head]) == EMPTY_SLOT) {
        // Queue is empty, wait for an element to be pushed
        Sleep(CONSUMER_TIMEOUT_MS);
    }
    buffer[head] = EMPTY_SLOT;
    head = nextBufferElement(head, capacity);
    return currentElement;
}

UINT64 SPSCFastForwardQueue::getCapacity() {
    return capacity;
}
