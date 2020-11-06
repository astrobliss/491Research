#include "src/pch.h"
#include "MoodyCamelConcurrentQueue.h"

MoodyCamelConcurrentQueue::MoodyCamelConcurrentQueue(int maxCapacity) {
    queue = new moodycamel::BlockingConcurrentQueue<UINT64>(maxCapacity);
}

MoodyCamelConcurrentQueue::~MoodyCamelConcurrentQueue() {
    delete queue;
}

void MoodyCamelConcurrentQueue::push(UINT64 newElement) {
    while (!queue->try_enqueue(newElement)) {
        // Queue is empty, wait for an element to be pushed
        Sleep(PRODUCER_TIMEOUT_MS);
    }
}

UINT64 MoodyCamelConcurrentQueue::pop() {
    UINT64 currentElement;
    queue->wait_dequeue(currentElement);
    return currentElement;
}

UINT64 MoodyCamelConcurrentQueue::getCapacity() {
    return queue->size_approx();
}
