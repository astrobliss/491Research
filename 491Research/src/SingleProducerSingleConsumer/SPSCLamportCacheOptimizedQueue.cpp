#include "src/pch.h"
#include "SPSCLamportCacheOptimizedQueue.h"

SPSCLamportCacheOptimizedQueue::SPSCLamportCacheOptimizedQueue(int maxCapacity) : buffer(new UINT64[maxCapacity]) {
    if ((maxCapacity & (maxCapacity - 1)) != 0) {
        throw std::invalid_argument("SPSCFastForwardQueue requires maxCapacity to be a power of two");
    }
    localHead = localTail = headCopy = tailCopy = head = tail = 0;
    //mask = maxCapacity - 1;
    //buffer = new UINT64[maxCapacity];
}

SPSCLamportCacheOptimizedQueue::~SPSCLamportCacheOptimizedQueue() {
    delete[] buffer;
}

void SPSCLamportCacheOptimizedQueue::push(UINT64 &newElement) {
    int staleness = (localTail - tail + BIT_MASK) & BIT_MASK;
    if (staleness == MAX_STALENESS) {
        tail = localTail;
    }
    while (true) {
        register int nextSlot = ((localTail + 1) & BIT_MASK);
        // try with the copy
        if (nextSlot != headCopy) {
            // if its the n-th successful push maybe also pre-emptively try to read the value
            buffer[localTail] = newElement;
            localTail = nextSlot;
            return;
        }
        // failed, try to read from cache
        headCopy = head;
        for (int i = 0; i < NUM_RETRIES; i++) {
            if (nextSlot != headCopy) {
                buffer[localTail] = newElement;
                localTail = nextSlot;
                return;
            }
            // read from memory
            headCopy = head;
        }
    }
}

UINT64 SPSCLamportCacheOptimizedQueue::pop() {
    int staleness = (localHead - head + BIT_MASK) & BIT_MASK;
    if (staleness == MAX_STALENESS) {
        head = localHead;
    }
    while (true) {
        if (localHead != tailCopy) {
            // successful, if n-th successful pop maybe pre-emptively update tail
            UINT64 currentElement = buffer[localHead];
            localHead = ((localHead + 1) & BIT_MASK);
            return currentElement;
        }
        // failed, try to read from cache
        tailCopy = tail;
        while (true) {
            if (localHead != tailCopy) {
                UINT64 currentElement = buffer[localHead];
                localHead = ((localHead + 1) & BIT_MASK);
                return currentElement;
            }
            // read from memory
            tailCopy = tail;
        }
    }
}

UINT64 SPSCLamportCacheOptimizedQueue::getCapacity() {
    return -1;
}
