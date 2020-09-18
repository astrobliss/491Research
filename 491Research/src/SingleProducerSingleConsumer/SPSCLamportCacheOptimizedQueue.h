#pragma once
#include "ProducerConsumerQueue.h"
/**
* Single Producer Single Consumer Queue Originally designed by Lamport here https://doi.org/10.1145/69624.357207 But head/tail are kept on separate cachlines
* Benifits: The Producer and Consumer can both acces the buffer at the same time
* Downsides: The head and tail are used by both producer and consumer which can cause cache flushes
*/
class SPSCLamportCacheOptimizedQueue : public ProducerConsumerQueue {
private:
    static const int CONSUMER_TIMEOUT_MS = 50;
    static const int PRODUCER_TIMEOUT_MS = 50;
    static const int CACHELINE_SIZE_OVERESTIMATE = 1 << 15;
    UINT64* buffer;
    int* oversizedCacheLine = new int[CACHELINE_SIZE_OVERESTIMATE];
    int& head = *oversizedCacheLine;
    int& tail = *(oversizedCacheLine + CACHELINE_SIZE_OVERESTIMATE - 1);
    int capacity;
public:
    /**
    * Constructs a SPSCLamportQueue ProducerConsumerQueue implimentation with the specified maximum capicity
    */
    SPSCLamportCacheOptimizedQueue(int maxCapacity);
    ~SPSCLamportCacheOptimizedQueue();
    void push(UINT64);
    UINT64 pop();
    UINT64 getCapacity();
};