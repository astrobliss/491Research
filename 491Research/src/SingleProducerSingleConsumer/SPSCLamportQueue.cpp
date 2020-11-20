#include "src/pch.h"
#include "SPSCLamportQueue.h"

SPSCLamportQueue::SPSCLamportQueue(int maxCapacity) {
    if ((maxCapacity & (maxCapacity - 1)) != 0) {
        throw std::invalid_argument("SPSCLamportQueue requires maxCapacity to be a power of two");
    }

    queueFull = CreateSemaphore(nullptr, 0, 1, nullptr);
    queueEmpty = CreateSemaphore(nullptr, 0, 1, nullptr);
    head = tail = 0;
    capacity = maxCapacity;
    bufferSizeMask = capacity - 1;
    buffer = new UINT64[capacity];
}

SPSCLamportQueue::~SPSCLamportQueue() {
    delete[] buffer;
}

void SPSCLamportQueue::push(UINT64 newElement) {
    int nextSlot = ((tail + 1) & bufferSizeMask);
    while (nextSlot == head) {
        // the queue appears full, try a few times to see if it empties
        for (int retries = 0; retries < MAX_RETRIES; ++retries) {
            if (nextSlot != head) {
                buffer[tail] = newElement;
                tail = nextSlot;
                return;
            }
        }
        // synchronize with the other thread operating on this queue
        ReleaseSemaphore(queueEmpty, 1, nullptr);
        WaitForSingleObject(queueFull, 1);
    }
    buffer[tail] = newElement;
    tail = nextSlot;
}

UINT64 SPSCLamportQueue::pop() {
    while (head == tail) {
        // Queue is empty, wait for an element to be pushed
        for (int retries = 0; retries < MAX_RETRIES; ++retries) {
            if (head != tail) {
                UINT64 currentElement = buffer[head];
                head = ((head + 1) & bufferSizeMask);
                return currentElement;
            }
        }
        ReleaseSemaphore(queueFull, 1, nullptr);
        WaitForSingleObject(queueEmpty, 1);
    }
    UINT64 currentElement = buffer[head];
    head = ((head + 1) & bufferSizeMask);
    return currentElement;
}

UINT64 SPSCLamportQueue::getCapacity() {
    return capacity;
}
