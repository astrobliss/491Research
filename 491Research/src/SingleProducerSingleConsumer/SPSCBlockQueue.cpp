#include "src/pch.h"
#include "SPSCBlockQueue.h"

SPSCBlockQueue::SPSCBlockQueue(int maxCapacity) {
    if ((maxCapacity & (maxCapacity - 1)) != 0) {
        throw std::invalid_argument("SPSCBlockQueue requires maxCapacity to be a power of two");
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

SPSCBlockQueue::~SPSCBlockQueue() {
    delete[] buffer;
}

void SPSCBlockQueue::push(UINT64 newElement) {
    int nextSlot = ((tail + 1) & tailBitMask);
    if (buffer[tail] == EMPTY_SLOT) {
        buffer[tail] = newElement;
        tail = nextSlot;
    }
    else {
        volatile UINT64* volatileBuffer = buffer;
        InterlockedExchange(blocked + PRODUCER, 1);
        while (blocked[PRODUCER] || volatileBuffer[tail] != EMPTY_SLOT);
        blocked[PRODUCER] = 0;
        volatileBuffer[tail] = newElement;
        tail = nextSlot;
    }
    if (blocked[CONSUMER]) {
        InterlockedExchange(blocked + CONSUMER, 0);
    }
}

UINT64 SPSCBlockQueue::pop() {
    UINT64 currentElement;
    if ((currentElement = buffer[head]) != EMPTY_SLOT) {
        buffer[head] = EMPTY_SLOT;
        head = ((head + 1) & headBitMask);
    }
    else {
        //printf("reader blocked\n");
        volatile UINT64 *volatileBuffer = buffer;
        InterlockedExchange(blocked + CONSUMER, 1);
        while (blocked[CONSUMER] || (currentElement=volatileBuffer[head]) == EMPTY_SLOT);
        blocked[CONSUMER] = 0;
        volatileBuffer[head] = EMPTY_SLOT;
        head = ((head + 1) & headBitMask);
        //printf("reader freed\n");
    }
    if (blocked[PRODUCER]) {
        InterlockedExchange(blocked + PRODUCER, 0);
    }
    return currentElement;
}

UINT64 SPSCBlockQueue::getCapacity() {
    return capacity;
}
