#pragma once
#include "ProducerConsumerQueue.h"
/**
* Single Producer Single Consumer Queue originally designed by Lamport here https://doi.org/10.1145/69624.357207
* Benifits: The Producer and Consumer can both acces the buffer at the same time
* Downsides: The head and tail integer are likely placed next to eachother in memory which can lead to cache thrashing
*/
class SPSCLamportQueue {
private:
    HANDLE queueFull, queueEmpty;
    const int MAX_RETRIES = 50;
    UINT64* buffer;
    int head;
    int tail;
    char spacer3[128];
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
