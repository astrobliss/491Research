#include "src/pch.h"
#include "SPSCLamportCacheOptimizedQueue.h"

SPSCLamportCacheOptimizedQueue::SPSCLamportCacheOptimizedQueue(int maxCapacity) : buffer(new UINT64[maxCapacity]) {
    if ((maxCapacity & (maxCapacity - 1)) != 0) {
        throw std::invalid_argument("SPSCFastForwardQueue requires maxCapacity to be a power of two");
    }
    localHead = localTail = headCopy = tailCopy = head = tail = 0;
}

SPSCLamportCacheOptimizedQueue::~SPSCLamportCacheOptimizedQueue() {
    delete[] buffer;
}

void SPSCLamportCacheOptimizedQueue::push(UINT64 &newElement) {
    register const int nextSlot = ((localTail + 1) & BIT_MASK);
    while (true) {
        if (nextSlot != headCopy) {
            buffer[localTail] = newElement;
            tail = localTail = nextSlot;
            return;
        }
        // thread doesn't see free space, update from memory
        headCopy = head;
    }
}

UINT64 SPSCLamportCacheOptimizedQueue::pop() {
    while (true) {
        if (localHead != tailCopy) {
            UINT64 currentElement = buffer[localHead];
            head = localHead = ((localHead + 1) & BIT_MASK);
            return currentElement;
        }
        // thread doesn't see an element, update from memory
        tailCopy = tail;
    }
}

UINT64 SPSCLamportCacheOptimizedQueue::getCapacity() {
    return BIT_MASK + 1;
}
