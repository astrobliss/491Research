#pragma once
#include "ProducerConsumerQueue.h"
/**
* Single Producer Single Consumer Queue originally designed by Lamport here https://doi.org/10.1145/69624.357207
* Benifits: The Producer and Consumer can both acces the buffer at the same time
* Downsides: The head and tail integer are likely placed next to eachother in memory which can lead to cache thrashing
*/
class SPSCLamportQueue : public ProducerConsumerQueue {
private:
    static const int CONSUMER_TIMEOUT_MS = 1;
    static const int PRODUCER_TIMEOUT_MS = 1;
    UINT64* buffer;
    int head, tail;
    int bufferSizeMask;
    int capacity;
public:
    /**
    * Constructs a SPSCLamportQueue ProducerConsumerQueue implimentation with the specified maximum capicity which must be a power of two
    */
    SPSCLamportQueue(int maxCapacity);
    ~SPSCLamportQueue();
    void push(UINT64);
    UINT64 pop();
    UINT64 getCapacity();
};
