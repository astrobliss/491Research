#include "pch.h"
#define EMPTY -1
#define FULL 1
#define NORMAL 0
/**
* Single Producer Single Consumer Queue Originally designed by Giacomi Et Al here https://doi.org/10.1145/1345206.1345215
* Benifits: The head and tail are kept on separate cache lines and the cache lines are not shared between producer and consumer
* Downsides: A single Queue value must be kept to indicate empty array elements, used here is EMPTY_SLOT as -1
*/
class MutexQueue {
private:
    static const UINT64 EMPTY_SLOT = -1;
    UINT64* buffer;
    CONDITION_VARIABLE queueFull, queueEmpty;
    CRITICAL_SECTION blocked;
    LONG queueState = -1;
    int head;
    int headBitMask;
    char spacer[128];
    int tail;
    int tailBitMask;
    int capacity;
public:
    /**
    * Constructs a SPSCFastForwardQueue ProducerConsumerQueue implimentation with the specified maximum capicity, must be a power of 2
    * head and tail are now elements on either side of the oversizedCacheLine array to prevent them from being cached together
    */
    MutexQueue(int maxCapacity);
    ~MutexQueue();
    void push(UINT64);
    UINT64 pop();
    UINT64 getCapacity();
};