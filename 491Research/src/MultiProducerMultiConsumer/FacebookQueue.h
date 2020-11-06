#pragma once
#include "ProducerConsumerQueue.h"
class FacebookQueue {
private:
    folly::MPMCQueue<UINT64> *queue;
public:
    /**
    * Constructs the default Facebook MPMCQueue, input paramater is not used
    */
    FacebookQueue(int maxCapacity);
    ~FacebookQueue();
    void push(UINT64);
    UINT64 pop();
    UINT64 getCapacity();
};

