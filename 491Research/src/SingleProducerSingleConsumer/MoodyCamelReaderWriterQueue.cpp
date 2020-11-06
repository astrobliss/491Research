#include "src/pch.h"
#include "MoodyCamelReaderWriterQueue.h"

MoodyCamelReaderWriterQueue::MoodyCamelReaderWriterQueue(int maxCapacity) {
    queue = new moodycamel::BlockingReaderWriterQueue<UINT64>(maxCapacity);
}

MoodyCamelReaderWriterQueue::~MoodyCamelReaderWriterQueue() {
    delete queue;
}

void MoodyCamelReaderWriterQueue::push(UINT64 newElement) {
    while (!queue->try_enqueue(newElement)) {
        // Queue is empty, wait for an element to be pushed
        Sleep(PRODUCER_TIMEOUT_MS);
    }
}

UINT64 MoodyCamelReaderWriterQueue::pop() {
    UINT64 currentElement;
    queue->wait_dequeue(currentElement);
    return currentElement;
}

UINT64 MoodyCamelReaderWriterQueue::getCapacity() {
    return queue->size_approx();
}
