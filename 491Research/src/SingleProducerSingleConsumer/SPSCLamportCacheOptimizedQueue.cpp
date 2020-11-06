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
    if (nextSlot != head) {
        buffer[tail] = newElement;
        tail = nextSlot;
    }
    else {
        printf("writer blocked\n");
        volatile UINT64 volatileNextSlot = ((tail + 1) & tailBitMask);
        InterlockedExchange(&prodBlocked, 1);
        while (volatileNextSlot == head);
        buffer[tail] = newElement;
        tail = nextSlot;
        printf("writer freed\n");
    }
    if (!consBlocked) {
        return;
    }
    else {
        InterlockedExchange(&consBlocked, 0);
    }
}

UINT64 SPSCLamportCacheOptimizedQueue::pop() {
    UINT64 currentElement;
    if (head != tail) {
        currentElement = buffer[head];
        head = ((head + 1) & headBitMask);
    }
    else {
        printf("reader blocked\n");
        volatile UINT64 volatileTail = tail;
        InterlockedExchange(&consBlocked, 1);
        while (head == volatileTail);
        currentElement = buffer[head];
        head = ((head + 1) & headBitMask);
        printf("reader freed\n");
    }
    if (!prodBlocked) {
        return currentElement;
    }
    else {
        InterlockedExchange(&prodBlocked, 0);
        return currentElement;
    }
}

UINT64 SPSCLamportCacheOptimizedQueue::getCapacity() {
    return capacity;
}
