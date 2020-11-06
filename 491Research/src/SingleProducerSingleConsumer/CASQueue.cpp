#include "src/pch.h"
#include "CASQueue.h"

CASQueue::CASQueue(int maxCapacity) {
    if ((maxCapacity & (maxCapacity - 1)) != 0) {
        throw std::invalid_argument("SPSCFastForwardQueue requires maxCapacity to be a power of two");
    }
    queueState = EMPTY;
    queueEmpty = CreateSemaphore(NULL, 0, 1, NULL);
    queueFull = CreateSemaphore(NULL, 0, 1, NULL);
    if ((queueEmpty == nullptr) || (queueFull == nullptr)) {
        throw std::system_error(1, std::system_category(), "Unable to create semaphore synchronization variables");
    }
    capacity = maxCapacity;
    headBitMask = tailBitMask = capacity - 1;
    head = tail = 0;
    buffer = new UINT64[capacity];
    memset(buffer, EMPTY_SLOT, sizeof(UINT64) * capacity);
}

CASQueue::~CASQueue() {
    delete[] buffer;
}

void CASQueue::push(UINT64 newElement) {
    int nextSlot = ((tail + 1) & tailBitMask);
    if (buffer[tail] != EMPTY_SLOT) {
        // Queue is full, sleep until problem fixed
        if (InterlockedCompareExchange(&queueState, FULL, NORMAL) == NORMAL) {
            // Successfully communicated queue was full, if that was wrong undo otw sleep
            if (buffer[tail] != EMPTY_SLOT) {
                WaitForSingleObject(queueFull, INFINITE);
            }
            else {
                InterlockedCompareExchange(&queueState, NORMAL, FULL);
            }
        }
        else {
            // Unsuccessfully communicated queue was full, if you were correct override otw continue
            if (buffer[tail] != EMPTY_SLOT) {
                InterlockedExchange(&queueState, FULL);
                ReleaseSemaphore(queueEmpty, 1, nullptr);
                WaitForSingleObject(queueFull, INFINITE);
            }
        }
    }
    buffer[tail] = newElement;
    tail = nextSlot;
    if (queueState == EMPTY) {
        InterlockedExchange(&queueState, NORMAL);
        queueState = NORMAL;
        ReleaseSemaphore(queueEmpty, 1, nullptr);
    }
}

UINT64 CASQueue::pop() {
    UINT64 currentElement;
    while ((currentElement = buffer[head]) == EMPTY_SLOT) {
        // Queue is empty, sleep until problem fixed
        if (InterlockedCompareExchange(&queueState, EMPTY, NORMAL) == NORMAL) {
            // Successfully communicated queue was empty, if that was wrong undo otw sleep
            if ((currentElement = buffer[head]) == EMPTY_SLOT) {
                WaitForSingleObject(queueEmpty, INFINITE);
            }
            else {
                InterlockedCompareExchange(&queueState, NORMAL, EMPTY);
            }
        }
        else {
            // Unsuccessfully communicated queue was empty, if you were correct override otw continue
            if ((currentElement = buffer[head]) == EMPTY_SLOT) {
                //InterlockedExchange(&queueState, EMPTY);
                queueState = EMPTY;
                ReleaseSemaphore(queueFull, 1, nullptr);
                WaitForSingleObject(queueEmpty, INFINITE);
            }
        }
    }
    buffer[head] = EMPTY_SLOT;
    head = ((head + 1) & headBitMask);
    if (queueState == FULL) {
        //InterlockedExchange(&queueState, NORMAL);
        queueState = NORMAL;
        ReleaseSemaphore(queueFull, 1, nullptr);
    }
    return currentElement;
}

UINT64 CASQueue::getCapacity() {
    return capacity;
}
