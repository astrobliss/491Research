#pragma once
#include "src/pch.h"
#define SPIN_COUNT 50			// fastest speed with 50: 1-1 queue reaches 250M/sec on a single core

//class SWQueue {
//protected:
//	void** buf; //ring buffer
//	HANDLE eventExit;
//	UINT64 bufSize; //maximum capacity
//
//	int head;
//	char filler[128];
//	int tail;
//
//public:
//	SWQueue(int bufSizePower, HANDLE evenExit);
//	~SWQueue();
//
//	virtual bool push(void* item) = 0;
//	virtual void* pop(void) = 0;
//};


class SWQueueMulti {
	void** buf; //ring buffer
	HANDLE eventExit;
	DWORD mask;

	DWORD head;
	DWORD tail;

	CRITICAL_SECTION csProd, csCons;
public:
	SWQueueMulti(int bufSizePower);
	~SWQueueMulti();

	void push(void* item);
	void push(void** item, int nItems);
	void* pop(void);
};

class SWQueueOneToOne {
	HANDLE prodSema, consSema;
	UINT64* buf; //ring buffer
	DWORD mask;
	DWORD head;
	char buff[128];
	DWORD tail;

public:
	SWQueueOneToOne(int bufSizePower);
	~SWQueueOneToOne();

	void push(UINT64 item);
	UINT64 pop(void);
};



class IRLqueue {
	int THREADS;
	DWORD TLSidx, thisIdx;

	static DWORD totalIRLQueues;					// initialized in .cpp

	void** bufP, ** bufC;							// multiple ring buffers
	DWORD mask, bufSizePower;
	int nConsumers, nProducers;

	DWORD* headP, * headC;
	char buf[128];
	DWORD* tailP, * tailC;

	DWORD lastHelperIndexC = 0;						// last consumer with which the helper interacted
	HANDLE helperSemaProducer, helperSemaConsumer;
	HANDLE producerSleep, consumerSleep;

	HANDLE* producerSema, * consumerSema;
public:
	LONG nSeenProducers = 0, nSeenConsumers = 0;
	bool quitC, quitP;

	IRLqueue(int nProducers, int nConsumers, int bufSizePower);
	~IRLqueue();

	//int* f(void);
	void push(void* item);
	//void push(int threadID, void **item, int nItems);			// batch mode, not implemented
	bool pop(void** item);

	void RunHelper(void);
	void DumpMultipleItems(void** source, DWORD size);
};

