#include "src/pch.h"
#include "SemaBlockQueue.h"

SemaBlockQueue::SemaBlockQueue(int maxCapacity) {
    if ((maxCapacity & (maxCapacity - 1)) != 0) {
        throw std::invalid_argument("SemaBlockQueue requires maxCapacity to be a power of two");
    }
    capacity = maxCapacity;
    headBitMask = tailBitMask = capacity - 1;
    head = tail = 0;
    buffer = new UINT64[capacity];
    // Consider using memset
    for (int i = 0; i < capacity; ++i) {
        buffer[i] = EMPTY_SLOT;
    }
    producerBlockedSemaphore = CreateSemaphore(nullptr, 0, 1, nullptr);
    consumerBlockedSemaphore = CreateSemaphore(nullptr, 0, 1, nullptr);
}

SemaBlockQueue::~SemaBlockQueue() {
    delete[] buffer;
}

void SemaBlockQueue::push(UINT64 newElement) {
    int nextSlot = ((tail + 1) & tailBitMask);
    if (buffer[tail] == EMPTY_SLOT) {
        buffer[tail] = newElement;
        tail = nextSlot;
    }
    else {
        //printf("writer blocked\n");
        InterlockedExchange(blocked + PRODUCER, 1);
        if (blocked[CONSUMER] && static_cast<volatile UINT64*>(buffer)[static_cast<volatile int>(head)] != EMPTY_SLOT) {
            // CONSUMER has made no changes since being blocked, producer has total information because of double LOCK
            blocked[CONSUMER] = 0;
            ReleaseSemaphore(consumerBlockedSemaphore, 1, nullptr);
        }
        WaitForSingleObject(producerBlockedSemaphore, INFINITE);
        buffer[tail] = newElement;
        tail = nextSlot;
        //printf("writer freed\n");
    }
    if (blocked[CONSUMER]) {
        blocked[CONSUMER] = 0;
        ReleaseSemaphore(consumerBlockedSemaphore, 1, nullptr);
    }
}


UINT64 SemaBlockQueue::pop() {
    UINT64 currentElement;
    if ((currentElement = buffer[head]) != EMPTY_SLOT) {
        buffer[head] = EMPTY_SLOT;
        head = ((head + 1) & headBitMask);
    }
    else {
        //printf("reader blocked\n");
        InterlockedExchange(blocked + CONSUMER, 1);
        if (blocked[PRODUCER] && buffer[tail] == EMPTY_SLOT) {
            // PRODUCER has made no changes since being blocked, consumer has total information because of double LOCK
            blocked[PRODUCER] = 0;
            ReleaseSemaphore(producerBlockedSemaphore, 1, nullptr);
        }
        WaitForSingleObject(consumerBlockedSemaphore, INFINITE);
        currentElement = static_cast<volatile UINT64*>(buffer)[head];
        buffer[head] = EMPTY_SLOT;
        head = ((head + 1) & headBitMask);
        //printf("reader freed\n");
    }
    if (blocked[PRODUCER]) {
        blocked[PRODUCER] = 0;
        ReleaseSemaphore(producerBlockedSemaphore, 1, nullptr);
    }
    return currentElement;
}

UINT64 SemaBlockQueue::getCapacity() {
    return capacity;
}
