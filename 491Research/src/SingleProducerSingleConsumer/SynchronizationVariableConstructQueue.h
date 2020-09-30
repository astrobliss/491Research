#pragma once
#include "ProducerConsumerQueue.h"
/**
* Similar to the LamportQueue but here instead of sleeping then attempting to reaccess, this version uses a Condition Variable to sleep
* Benifits: The Producer and Consumer can both acces the buffer at the same time
* Downsides: The head and tail integer are likely placed next to eachother in memory which can lead to cache thrashing
*/
class SynchronizationVariableConstructQueue : public ProducerConsumerQueue {
private:
    CONDITION_VARIABLE queueEmpty, queueFull;
    bool consumerWaiting, producerWaiting;
    CRITICAL_SECTION unused;
    UINT64* buffer;
    int head, tail;
    int bufferSizeMask;
    int capacity;
public:
    /**
    * Constructs a SynchronizationVariableConstructQueue ProducerConsumerQueue implimentation with the specified maximum capicity which must be a power of two
    */
    SynchronizationVariableConstructQueue(int maxCapacity);
    ~SynchronizationVariableConstructQueue();
    void push(UINT64);
    UINT64 pop();
    UINT64 getCapacity();
};
