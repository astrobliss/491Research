#include "src/pch.h"
#include "MSFTConcurrentQueue.h"

MSFTConcurrentQueue::MSFTConcurrentQueue(int unused) {
}

MSFTConcurrentQueue::~MSFTConcurrentQueue() {
}

void MSFTConcurrentQueue::push(UINT64 newElement) {
    queue.push(newElement);
}

UINT64 MSFTConcurrentQueue::pop() {
    UINT64 currentElement;
    while (!queue.try_pop(currentElement)) {
        // Queue is empty, wait for an element to be pushed
        Sleep(CONSUMER_TIMEOUT_MS);
    }
    return currentElement;
}

UINT64 MSFTConcurrentQueue::getCapacity() {
    return queue.unsafe_size();
}
