#include "src/pch.h"
#include "SemaEventQueue.h"

SemaEventQueue::SemaEventQueue(int maxCapacity) {
    if ((maxCapacity & (maxCapacity - 1)) != 0) {
        throw std::invalid_argument("SemaEventQueue requires maxCapacity to be a power of two");
    }
    queueState = NORMAL;
    queueFull = CreateSemaphore(nullptr, 0, 1, nullptr);
    queueEmpty = CreateSemaphore(nullptr, 0, 1, nullptr);
    InitializeCriticalSection(&blocked);
    capacity = maxCapacity;
    headBitMask = tailBitMask = capacity - 1;
    head = tail = 0;
    buffer = new UINT64[capacity];
    memset(buffer, EMPTY_SLOT, sizeof(UINT64) * capacity);
}

SemaEventQueue::~SemaEventQueue() {
    delete[] buffer;
    CloseHandle(queueFull);
    CloseHandle(queueEmpty);
}

void SemaEventQueue::push(UINT64 newElement) {
    int nextSlot = ((tail + 1) & tailBitMask);
    while (buffer[tail] != EMPTY_SLOT) {
        // the queue appears full, try a few times to see if it empties
        for (int retries = 0; retries < MAX_RETRIES; ++retries) {
            if (buffer[tail] == EMPTY_SLOT) {
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

UINT64 SemaEventQueue::pop() {
    UINT64 currentElement;
    while ((currentElement = (buffer)[head]) == EMPTY_SLOT) {
        // Queue is empty, wait for an element to be pushed
        for (int retries = 0; retries < MAX_RETRIES; ++retries) {
            if ((currentElement = (buffer)[head]) != EMPTY_SLOT) {
                buffer[head] = EMPTY_SLOT;
                head = ((head + 1) & headBitMask);
                return currentElement;
            }
        }
        ReleaseSemaphore(queueFull, 1, nullptr);
        WaitForSingleObject(queueEmpty, 1);
    }
    buffer[head] = EMPTY_SLOT;
    head = ((head + 1) & headBitMask);
    return currentElement;
}

UINT64 SemaEventQueue::getCapacity() {
    return capacity;
}

bool SemaEventQueue::tryPush(UINT64 newElement) {
    int nextSlot = ((tail + 1) & tailBitMask);
    int x = 0;
    if (buffer[tail] != EMPTY_SLOT) {
        return false;
    }
    buffer[tail] = newElement;
    tail = nextSlot;
    return true;
}

bool SemaEventQueue::tryPop(UINT64& output) {
    int x = 0;
    if ((output = (buffer)[head]) == EMPTY_SLOT) {
        return false;
    }
    buffer[head] = EMPTY_SLOT;
    head = ((head + 1) & headBitMask);
    return true;
}

void SemaEventQueue::prodSync() {
    ReleaseSemaphore(queueEmpty, 1, nullptr);
    WaitForSingleObject(queueFull, 1);
}

void SemaEventQueue::freeProd() {
    ReleaseSemaphore(queueFull, 1, nullptr);
}

void SemaEventQueue::consSync() {
    ReleaseSemaphore(queueFull, 1, nullptr);
    WaitForSingleObject(queueEmpty, 1);
}

void SemaEventQueue::freeCons() {
    ReleaseSemaphore(queueEmpty, 1, nullptr);
}
