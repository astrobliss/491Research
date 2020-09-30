#include "src/pch.h"
#include "SynchronizationVariableConstructQueue.h"

SynchronizationVariableConstructQueue::SynchronizationVariableConstructQueue(int maxCapacity) {
    if ((maxCapacity & (maxCapacity - 1)) != 0) {
        throw std::invalid_argument("SPSCFastForwardQueue requires maxCapacity to be a power of two");
    }
    head = tail = 0;
    capacity = maxCapacity;
    consumerWaiting = false;
    producerWaiting = false;
    InitializeConditionVariable(&queueEmpty);
    InitializeConditionVariable(&queueFull);
    InitializeCriticalSection(&unused);
    bufferSizeMask = capacity - 1;
    buffer = new UINT64[capacity];
}

SynchronizationVariableConstructQueue::~SynchronizationVariableConstructQueue() {
    delete[] buffer;
}

void SynchronizationVariableConstructQueue::push(UINT64 newElement) {
    int nextSlot = ((tail + 1) & bufferSizeMask);
    while (nextSlot == head) {
        // Queue is full, wait for an element to be popped
        producerWaiting = true;
        EnterCriticalSection(&unused);
        SleepConditionVariableCS(&queueFull, &unused, INFINITE);
    }
    buffer[nextSlot] = newElement;
    tail = nextSlot;
    if (consumerWaiting) {
        WakeConditionVariable(&queueEmpty);
        producerWaiting = false;
    }
}

UINT64 SynchronizationVariableConstructQueue::pop() {
    while (head == tail) {
        // Queue is empty, wait for an element to be pushed
        consumerWaiting = true;
        EnterCriticalSection(&unused);
        SleepConditionVariableCS(&queueEmpty, &unused, INFINITE);
    }
    UINT64 currentElement = buffer[head];
    head = ((head + 1) & bufferSizeMask);
    if (producerWaiting) {
        WakeConditionVariable(&queueFull);
        producerWaiting = false;
    }
    return currentElement;
}

UINT64 SynchronizationVariableConstructQueue::getCapacity() {
    return capacity;
}
