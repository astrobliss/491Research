#include "src/pch.h"
#include "SPSCFastForwardQueue.h"

SPSCFastForwardQueue::SPSCFastForwardQueue(int maxCapacity) {
    if ((maxCapacity & (maxCapacity - 1)) != 0) {
        throw std::invalid_argument("SPSCFastForwardQueue requires maxCapacity to be a power of two");
    }
    capacity = maxCapacity;
    headBitMask = tailBitMask = capacity - 1;
    head = tail = 0;
    buffer = new UINT64[capacity];
    // Consider using memset
    for (int i = 0; i < capacity; ++i) {
        buffer[i] = EMPTY_SLOT;
    }
}

SPSCFastForwardQueue::~SPSCFastForwardQueue() {
    delete[] buffer;
}

void SPSCFastForwardQueue::push(UINT64 newElement) {
    int nextSlot = ((tail + 1) & tailBitMask);
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
    head = ((head + 1) & headBitMask);
    return currentElement;
}

UINT64 SPSCFastForwardQueue::getCapacity() {
    return capacity;
}
