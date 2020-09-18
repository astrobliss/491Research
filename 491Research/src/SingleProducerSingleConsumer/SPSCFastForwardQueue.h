#pragma once
#include "ProducerConsumerQueue.h"
/**
* Single Producer Single Consumer Queue Originally designed by Giacomi Et Al here https://doi.org/10.1145/1345206.1345215
* Benifits: The head and tail are kept on separate cache lines and the cache lines are not shared between producer and consumer
* Downsides: A single Queue value must be kept to indicate empty array elements, used here is EMPTY_SLOT as -1
*/
class SPSCFastForwardQueue : public ProducerConsumerQueue {
private:
    static const int CONSUMER_TIMEOUT_MS = 50;
    static const int PRODUCER_TIMEOUT_MS = 50;
    static const UINT64 EMPTY_SLOT = -1;
    static const int CACHELINE_SIZE_OVERESTIMATE = 1 << 15;
    UINT64* buffer;
    int* oversizedCacheLine = new int[CACHELINE_SIZE_OVERESTIMATE];
    int& head = *oversizedCacheLine;
    int& tail = *(oversizedCacheLine + CACHELINE_SIZE_OVERESTIMATE - 1);
    int capacity;
public:
    /**
    * Constructs a SPSCLamportQueue ProducerConsumerQueue implimentation with the specified maximum capicity
    * head and tail are now elements on either side of the oversizedCacheLine array to prevent them from being cached together
    */
    SPSCFastForwardQueue(int maxCapacity);
    ~SPSCFastForwardQueue();
    void push(UINT64);
    UINT64 pop();
    UINT64 getCapacity();
};