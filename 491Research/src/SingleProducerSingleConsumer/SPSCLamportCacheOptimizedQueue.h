#pragma once
#include "ProducerConsumerQueue.h"
/**
* Single Producer Single Consumer Queue Originally designed by Lamport here https://doi.org/10.1145/69624.357207 But head/tail are kept on separate cachlines
* Benifits: The Producer and Consumer can both acces the buffer at the same time
* Downsides: The head and tail are used by both producer and consumer which can cause cache flushes, the head and tail are on separate cache lines
*/
class SPSCLamportCacheOptimizedQueue : public ProducerConsumerQueue {
private:
    static const int CONSUMER_TIMEOUT_MS = 1;
    static const int PRODUCER_TIMEOUT_MS = 1;
    UINT64* buffer;
    int head;
    int headBitMask;
    char spacer[128];
    int tail;
    int tailBitMask;
    int capacity;
public:
    /**
    * Constructs a SPSCLamportQueue ProducerConsumerQueue implimentation with the specified maximum capicity which must be a power of two
    */
    SPSCLamportCacheOptimizedQueue(int maxCapacity);
    ~SPSCLamportCacheOptimizedQueue();
    void push(UINT64);
    UINT64 pop();
    UINT64 getCapacity();
};