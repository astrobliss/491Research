#pragma once
#include "ProducerConsumerQueue.h"
class MoodyCamelReaderWriterQueue {
private:
    moodycamel::BlockingReaderWriterQueue<UINT64>* queue;
    static const int PRODUCER_TIMEOUT_MS = 10;
public:
    /**
    * Constructs the default MoodyCamel Blocking Queue
    */
    MoodyCamelReaderWriterQueue(int maxCapicity);
    ~MoodyCamelReaderWriterQueue();
    void push(UINT64);
    UINT64 pop();
    UINT64 getCapacity();
};
