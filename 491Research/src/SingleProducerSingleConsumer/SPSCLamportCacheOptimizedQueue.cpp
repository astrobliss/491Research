#include "src/pch.h"
#include "SPSCLamportCacheOptimizedQueue.h"
#include "src/Utilities/QueueUtilities.cpp"

SPSCLamportCacheOptimizedQueue::SPSCLamportCacheOptimizedQueue(int maxCapacity) {
    head = tail = 0;
    capacity = maxCapacity;
    buffer = new UINT64[capacity];
}

SPSCLamportCacheOptimizedQueue::~SPSCLamportCacheOptimizedQueue() {
    delete[] buffer;
    delete[] oversizedCacheLine;
}

void SPSCLamportCacheOptimizedQueue::push(UINT64 newElement) {
    int nextSlot = nextBufferElement(tail, capacity);
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
    head = nextBufferElement(head, capacity);
    return currentElement;
}

UINT64 SPSCLamportCacheOptimizedQueue::getCapacity() {
    return capacity;
}
