#pragma once
#include "ProducerConsumerQueue.h"
#include "pch.h"
class MSFTConcurrentQueue {
private:
    concurrency::concurrent_queue<UINT64> queue;
    static const int CONSUMER_TIMEOUT_MS = 1;
public:
    /**
    * Constructs the default Microsoft Concurrent Queue, input paramater is not used
    */
    MSFTConcurrentQueue(int unused);
    ~MSFTConcurrentQueue();
    void push(UINT64);
    UINT64 pop();
    UINT64 getCapacity();
};

