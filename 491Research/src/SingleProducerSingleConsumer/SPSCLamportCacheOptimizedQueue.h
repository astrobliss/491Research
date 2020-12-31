#pragma once
#define NUM_RETRIES (1<<30)
#define MAX_STALENESS (1<<10)
#define BIT_MASK (4095)
/**
* Single Producer Single Consumer Queue Originally designed by Lamport here https://doi.org/10.1145/69624.357207 But head/tail are kept on separate cachlines
* Benifits: The Producer and Consumer can both acces the buffer at the same time
* Downsides: The head and tail are used by both producer and consumer which can cause cache flushes, the head and tail are on separate cache lines
*/
class SPSCLamportCacheOptimizedQueue {
private:
	UINT64* const buffer;
	//const int mask;
	char buf0[128];
	volatile int head;
	char buf1[128];
	volatile int tail;
	char buf2[128];
	int headCopy;
	//char buf3[128];
	int localTail;
	char buf4[128];
	int tailCopy;
	//char buf5[128];
	int localHead;
public:
    /**
    * Constructs a SPSCLamportQueue ProducerConsumerQueue implimentation with the specified maximum capicity which must be a power of two
    */
    SPSCLamportCacheOptimizedQueue(int maxCapacity);
    ~SPSCLamportCacheOptimizedQueue();
    void push(UINT64&);
    UINT64 pop();
    UINT64 getCapacity();
};