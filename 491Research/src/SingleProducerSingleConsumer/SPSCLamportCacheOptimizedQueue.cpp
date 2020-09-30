#include "src/pch.h"
#include "SPSCLamportCacheOptimizedQueue.h"

SPSCLamportCacheOptimizedQueue::SPSCLamportCacheOptimizedQueue(int maxCapacity) {
    if ((maxCapacity & (maxCapacity - 1)) != 0) {
        throw std::invalid_argument("SPSCFastForwardQueue requires maxCapacity to be a power of two");
    }
    capacity = maxCapacity;
    head = tail = 0;
    headBitMask = tailBitMask = capacity - 1;
    buffer = new UINT64[capacity];
}

SPSCLamportCacheOptimizedQueue::~SPSCLamportCacheOptimizedQueue() {
    delete[] buffer;
}

void SPSCLamportCacheOptimizedQueue::push(UINT64 newElement) {
    int nextSlot = ((tail + 1) & tailBitMask);
    while (nextSlot == head) {
        // Queue is full, wait for an element to be popped
        Sleep(PRODUCER_TIMEOUT_MS);
    }
    buffer[nextSlot] = newElement;
    tail = nextSlot;
}

UINT64 SPSCLamportCacheOptimizedQueue::pop() {
    while (head == tail) {
        // Queue is empty, wait for an element to be pushed
        Sleep(CONSUMER_TIMEOUT_MS);
    }
    UINT64 currentElement = buffer[head];
    head = ((head + 1) & headBitMask);
    return currentElement;
}

UINT64 SPSCLamportCacheOptimizedQueue::getCapacity() {
    return capacity;
}
