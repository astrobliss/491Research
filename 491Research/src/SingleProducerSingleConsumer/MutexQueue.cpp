#include "src/pch.h"
#include "MutexQueue.h"

MutexQueue::MutexQueue(int maxCapacity) {
    if ((maxCapacity & (maxCapacity - 1)) != 0) {
        throw std::invalid_argument("SPSCFastForwardQueue requires maxCapacity to be a power of two");
    }
    queueState = NORMAL;
    InitializeCriticalSection(&blocked);
    capacity = maxCapacity;
    headBitMask = tailBitMask = capacity - 1;
    head = tail = 0;
    buffer = new UINT64[capacity];
    memset(buffer, EMPTY_SLOT, sizeof(UINT64) * capacity);
}

MutexQueue::~MutexQueue() {
    delete[] buffer;
}

void MutexQueue::push(UINT64 newElement) {
    int nextSlot = ((tail + 1) & tailBitMask);
    if (buffer[tail] != EMPTY_SLOT) {
        // Queue is full, sleep until problem fixed
        EnterCriticalSection(&blocked);
        while (buffer[tail] != EMPTY_SLOT) {
            LeaveCriticalSection(&blocked);
            InterlockedExchange(&queueState, FULL);

        }
        LeaveCriticalSection(&blocked);
    }
    buffer[tail] = newElement;
    tail = nextSlot;
    if (queueState == EMPTY) {
        InterlockedExchange(&queueState, NORMAL);
        WakeConditionVariable(&queueEmpty);
    }
}

UINT64 MutexQueue::pop() {
    UINT64 currentElement;
    if ((currentElement = buffer[head]) == EMPTY_SLOT) {
        // Queue is empty, sleep until problem fixed
        EnterCriticalSection(&blocked);
        while ((currentElement = buffer[head]) == EMPTY_SLOT) {
            InterlockedExchange(&queueState, EMPTY);
            WakeConditionVariable(&queueFull);
            SleepConditionVariableCS(&queueEmpty, &blocked, INFINITE);
        }
        LeaveCriticalSection(&blocked);
    }
    buffer[head] = EMPTY_SLOT;
    head = ((head + 1) & headBitMask);
    if (queueState == FULL) {
        InterlockedExchange(&queueState, NORMAL);
        WakeConditionVariable(&queueFull);
    }
    return currentElement;
}

UINT64 MutexQueue::getCapacity() {
    return capacity;
}
