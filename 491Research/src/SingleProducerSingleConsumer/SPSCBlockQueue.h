#pragma once
// Uses Intel Chipset guarantees to send updates to unblock threads
class SPSCBlockQueue {
private:
    static const int PRODUCER = 0;
    static const int CONSUMER = 1;
    static const UINT64 EMPTY_SLOT = -1;
    UINT64* buffer;
    int head;

    int headBitMask;
    char spacer1[128];
    int tail;
    int tailBitMask;
    int capacity;
    char spacer2[128];
    volatile LONG blocked[2] = { 0,0 };
public:
    /**
    * Constructs a SPSCFastForwardQueue ProducerConsumerQueue implimentation with the specified maximum capicity, must be a power of 2
    * head and tail are now elements on either side of the oversizedCacheLine array to prevent them from being cached together
    */
    SPSCBlockQueue(int maxCapacity);
    ~SPSCBlockQueue();
    void push(UINT64);
    UINT64 pop();
    UINT64 getCapacity();
};

