#include "src/pch.h"
#include "FacebookQueue.h"

FacebookQueue::FacebookQueue(int maxCapacity) {
    queue = new folly::MPMCQueue<UINT64>(maxCapacity);
}

FacebookQueue::~FacebookQueue() {
    delete queue;
}

void FacebookQueue::push(UINT64 newElement) {
    queue->blockingWrite(newElement);
}

UINT64 FacebookQueue::pop() {
    UINT64 currentElement;
    queue->blockingRead(currentElement);
    return currentElement;
}

UINT64 FacebookQueue::getCapacity() {
    return queue->size();
}

