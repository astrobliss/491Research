#pragma once
#include "ProducerConsumerQueue.h"
class MoodyCamelConcurrentQueue {
private:
    moodycamel::BlockingConcurrentQueue<UINT64> *queue;
    static const int PRODUCER_TIMEOUT_MS = 10;
public:
    /**
    * Constructs the default MoodyCamel Blocking Queue
    */
    MoodyCamelConcurrentQueue(int maxCapicity);
    ~MoodyCamelConcurrentQueue();
    void push(UINT64);
    UINT64 pop();
    UINT64 getCapacity();
};

