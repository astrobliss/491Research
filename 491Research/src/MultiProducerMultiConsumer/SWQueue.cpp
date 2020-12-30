#include "src/pch.h"
#include "SWQueue.h"

DWORD IRLqueue::totalIRLQueues = 0;
int min(int a, int b) {
	return (a < b) ? a : b;
}
DWORD WINAPI StartHelper(void* p) {
	//SetThreadAffinityMask (GetCurrentThread(), 1<<9);			// much slower with this
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	IRLqueue* iq = (IRLqueue*)p;
	iq->RunHelper();
	return 0;
}

IRLqueue::IRLqueue(int nProducers, int nConsumers, int bufSizePower) : nProducers(nProducers), nConsumers(nConsumers), bufSizePower(bufSizePower) {
	THREADS = (nProducers > nConsumers) ? nProducers : nConsumers;
	thisIdx = InterlockedIncrement(&totalIRLQueues) - 1;
	//printf("IRLqueue %p uses idx %d (size %d)\n", this, thisIdx, 1 << bufSizePower);

	//if ((TLSidx = TlsAlloc()) == TLS_OUT_OF_INDEXES)
	//	printf ("TlsAlloc failed with %d\n", GetLastError());

	if (bufSizePower <= 0) {
		printf("buffer size must be at least 2 items\n");
		exit(-1);
	}

	// buffer initialization
	bufP = new void* [(1 << bufSizePower) * nProducers + 128];
	bufC = new void* [(1 << bufSizePower) * nConsumers + 128];

	mask = (1 << bufSizePower) - 1;

	// producers
	headP = new DWORD[nProducers + 128];		// this stabilizes speed on Server 2019 & soccer
	tailP = new DWORD[nProducers + 128];
	//itemsIn = new UINT64[nProducers];
	memset(headP, 0, sizeof(*headP) * nProducers);
	memset(tailP, 0, sizeof(*tailP) * nProducers);

	// consumers
	headC = new DWORD[nConsumers + 128];
	tailC = new DWORD[nConsumers + 128];
	//itemsOut = new UINT64[nConsumers];
	memset(headC, 0, sizeof(*headC) * nConsumers);
	memset(tailC, 0, sizeof(*tailC) * nConsumers);

	producerSema = new HANDLE[THREADS];
	consumerSema = new HANDLE[THREADS];

	for (int i = 0; i < THREADS; i++) {
		producerSema[i] = CreateSemaphore(NULL, 0, 1, NULL);
		consumerSema[i] = CreateSemaphore(NULL, 0, 1, NULL);
	}

	helperSemaProducer = CreateSemaphore(NULL, 0, 1, NULL);
	helperSemaConsumer = CreateSemaphore(NULL, 0, 1, NULL);

	quitP = false, quitC = false;
	CreateThread(NULL, 0, StartHelper, this, 0, NULL);
}

IRLqueue::~IRLqueue() {
	//TlsFree(TLSidx);
	delete bufP;
	delete bufC;
	delete headP;
	delete tailP;
	delete headC;
	delete tailC;
}

void IRLqueue::push(void* item) {
	//thread_local int threadID = InterlockedIncrement (&nSeenProducers)-1;

	/*thread_local int* p = [&]		// lambda function
	{
		int* arr = new int[100];
		memset(arr, -1, sizeof(int) * 100);
		//arr[thisIdx] = InterlockedIncrement(&nSeenProducers)-1;
		return arr;
	} ();

	// Benchmarks on Server 2019 (kernel 17744) ---------------------------------
	// Buggy ver from Feb 2018: 560M/s (1+1)
	// Fastest version: TLS1 + TLS4 (534M/s 1+1)
	// Original: TLS2 + TLS4 (500M/s)
	// Strangely not optimal: TLS1 + TLS3 (520M/s)

	// ver TLS1
	int threadID;
	if (p[thisIdx] != -1)
		threadID = p[thisIdx];
	else
		threadID = p[thisIdx] = InterlockedIncrement(&nSeenProducers) - 1;
*/
	// ver TLS2
	//if (p[thisIdx] == -1) 
	//	p[thisIdx] = InterlockedIncrement(&nSeenProducers) - 1;
	//int threadID = p[thisIdx];

	//printf("producer thread %d uses queue %d\n", GetCurrentThreadId(), threadID);

	//UINT64 qID = (UINT64) TlsGetValue(TLSidx);
	//if (qID == 0)
	//{
	//	TlsSetValue(TLSidx, (LPVOID)++nSeenProducers);
	//	qID = nSeenProducers;
	//	printf("new producer %d: threadID %d\n", nSeenProducers, GetCurrentThreadId());
	//}
	int threadID = 0;
	void** bufStartPtr = bufP + (threadID << bufSizePower);

	while (true) {
		int count = 0;
		while (count < SPIN_COUNT) {
			// spins for a bit at full capacity
			register DWORD oldTail = tailP[threadID];
			register DWORD newTail = (oldTail + 1) & mask;
			if (newTail != headP[threadID])			// can deposit one item withouth overflowing the queue?
			{
				// empty slots available; add item
				bufStartPtr[oldTail] = item;
				_ReadWriteBarrier();		// compiler fence; do not reorder writes across this boundary
				tailP[threadID] = newTail;
				return;
			}
			count++;
		}

		ReleaseSemaphore(helperSemaProducer, 1, NULL);
		WaitForSingleObject(producerSema[threadID], 1);
	}
}

bool IRLqueue::pop(void** item) {
	//thread_local int threadID = InterlockedIncrement(&nSeenConsumers)-1;
	//thread_local int *p = f();
	/*
	thread_local int* p = [&]		// lambda function
	{
		int* arr = new int[100];
		//arr[thisIdx] = InterlockedIncrement(&nSeenConsumers) - 1;
		memset(arr, -1, sizeof(int) * 100);
		return arr;
	} ();

	// ver TLS3
	//int threadID;
	//if (p[thisIdx] != -1)
	//	threadID = p[thisIdx];
	//else
	//	threadID = p[thisIdx] = InterlockedIncrement(&nSeenConsumers) - 1;

	// ver TLS4
	if (p[thisIdx] == -1)
		p[thisIdx] = InterlockedIncrement(&nSeenConsumers) - 1;
		*/
	int threadID = 0;//p[thisIdx];

	void** bufStartPtr = bufC + (threadID << bufSizePower);

	while (true) {
		int count = 0;
		while (count < SPIN_COUNT) {
			DWORD oldHead = headC[threadID];
			if (oldHead != tailC[threadID]) {
				*item = bufStartPtr[oldHead];
				_ReadWriteBarrier();			// compiler fence; do not update the head before the read is finished
				headC[threadID] = (oldHead + 1) & mask;
				return true;
			}

			count++;
		}

		if (quitC) {
			return false;
		}

		ReleaseSemaphore(helperSemaConsumer, 1, NULL);
		WaitForSingleObject(consumerSema[threadID], 1);		// 1ms timeout needed to prevent deadlocks on exit
	}

	return false;
}

void IRLqueue::RunHelper(void) {
	UINT64 c = 0, last = 0;
	clock_t start = clock();

	while (true) {
		int count = 0;
		while (count < SPIN_COUNT) {
			// find an input queue with non-empty elements; finding the one with the most items hurts performance in 1+1
			for (DWORD readyIndex = 0; readyIndex < nProducers; readyIndex++) {
				register DWORD readyHead = headP[readyIndex];
				register DWORD readyTail = tailP[readyIndex];

				DWORD grab = (readyTail - readyHead + mask + 1) & mask;
				if (grab > 0) {
					void** bufStartPtr = bufP + (readyIndex << bufSizePower);

					// unwrap the data in one or two memcpies
					DWORD continuousSize = min(grab, mask + 1 - readyHead);
					DumpMultipleItems((void**)(bufStartPtr + readyHead), continuousSize);

					DWORD leftover = grab - continuousSize;
					if (leftover > 0)
						DumpMultipleItems((void**)bufStartPtr, leftover);

					_ReadWriteBarrier();
					headP[readyIndex] = readyTail;
					//ReleaseSemaphore(producerSema[readyIndex], 1, NULL);		// A: 1+1 gives 0.24M/sec; however, faster on 200+200, no affinity
					count = 0;
				}

				ReleaseSemaphore(producerSema[readyIndex], 1, NULL);		// B: 1+1 gives 0.51M/sec
			}

			count++;
		}

		if (quitP && !quitC) {
			//printf("helper signalling consumer\n");
			quitC = true;
		}

		WaitForSingleObject(helperSemaProducer, 1);
	}
}

inline void IRLqueue::DumpMultipleItems(void** source, DWORD size) {
	DWORD curPos = 0;		// position on input

	while (true) {
		int count = 0;
		while (count < SPIN_COUNT) {
			// finding an input queue with the most empty elements doesn't improve speed
			int checked = 0;
			for (DWORD readyIndex = lastHelperIndexC; checked++ < nConsumers; readyIndex = (readyIndex + 1) % nConsumers) {
				register DWORD readyHead = headC[readyIndex], readyTail = tailC[readyIndex];
				DWORD occupied = (readyTail - readyHead + mask + 1) & mask;
				register DWORD drop = mask - occupied;

				if (drop > 0) {
					DWORD dropReal = min(drop, size - curPos);		// don't overflow the input
					void** bufStartPtr = bufC + (readyIndex << bufSizePower);

					DWORD continuousSize = min(dropReal, mask + 1 - readyTail);
					memcpy(bufStartPtr + readyTail, source + curPos, continuousSize * sizeof(void*));

					DWORD leftover = dropReal - continuousSize;
					if (leftover > 0)
						memcpy(bufStartPtr, source + curPos + continuousSize, leftover * sizeof(void*));

					_ReadWriteBarrier();
					tailC[readyIndex] = (readyTail + dropReal) & mask;

					ReleaseSemaphore(consumerSema[readyIndex], 1, NULL);

					curPos += dropReal;
					if (curPos == size) {
						lastHelperIndexC = readyIndex;		//	continue from the same spot 
						//lastHelperIndexC = (readyIndex + 1) % nConsumers;
						return;
					}

					count = 0;
				}

				//ReleaseSemaphore(consumerSema[readyIndex], 1, NULL);
			}

			count++;
		}

		WaitForSingleObject(helperSemaConsumer, INFINITE);
	}
}


//--------------------------------------------------------------------
SWQueueMulti::SWQueueMulti(int bufSizePower) {
	// buffer initialization
	buf = new void* [1 << bufSizePower];
	mask = (1 << bufSizePower) - 1;

	head = 0;
	tail = 0;

	InitializeCriticalSection(&csProd);
	InitializeCriticalSection(&csCons);
}

SWQueueMulti::~SWQueueMulti() {
	delete buf;
	DeleteCriticalSection(&csProd);
	DeleteCriticalSection(&csCons);
}

void SWQueueMulti::push(void* item) {
	while (true) {
		int count = 0;
		while (count < SPIN_COUNT && 0) {
			// spins for a bit at full capacity
			if (((tail + 1) & mask) != head)			// can deposit one item withouth overflowing the queue?
				break;
			count++;
		}

		if (count < SPIN_COUNT) {
			// empty slots available
			EnterCriticalSection(&this->csProd);

			//add item
			register int newTail = (tail + 1) & mask;
			if (newTail != head) {
				buf[tail] = item;
				tail = newTail;
				LeaveCriticalSection(&csProd);
				return;
			}
			else
				LeaveCriticalSection(&csProd);
		}

		Sleep(1);
		//SwitchToThread();
	}
}

void SWQueueMulti::push(void** item, int nItems) {
	while (true) {
		int count = 0;
		// spins for a bit at full capacity
		//while (count < SPIN_COUNT && 0)
		//{
		//	int virtualHead = head;
		//	if (tail >= head)
		//		virtualHead += mask + 1;				// allows direct comparison to tail + nItems below

		//	if (tail + nItems < virtualHead)			// can deposit one item withouth overflowing the queue?
		//		break;

		//	count++;
		//}

		//if (count < SPIN_COUNT)
		{
			// empty slots available
			EnterCriticalSection(&this->csProd);

			//add item
			int virtualHead = head;
			if (tail >= head)
				virtualHead += mask + 1;				// allows direct comparison to tail + nItems below

			register int newTail = tail + nItems;
			if (newTail < virtualHead) {
				int grab = min(nItems, mask + 1 - tail);
				memcpy(buf + tail, item, sizeof(void*) * grab);

				int leftover = nItems - grab;
				if (leftover > 0)
					memcpy(buf, item + grab, sizeof(void*) * leftover);

				tail = newTail & mask;
				LeaveCriticalSection(&csProd);
				return;
			}
			else {
				LeaveCriticalSection(&csProd);
				Sleep(1);
				//SwitchToThread();
			}
		}
		//else
		//	// Sleep(1);
		//	SwitchToThread();
	}
}


void* SWQueueMulti::pop(void) {

	//if ((WaitForSingleObject(eventExit, 0)) == WAIT_OBJECT_0) {
	//	return NULL;
	//}

	while (true) {
		int count = 0;
		//while (head == tail && count < SPIN_COUNT) 
		//	count++;

		//if (count < SPIN_COUNT)
		{
			EnterCriticalSection(&this->csCons);

			if (head != tail) {
				void* temp = buf[head];
				this->head = (head + 1) & mask;
				//this->curSize--;
				LeaveCriticalSection(&csCons);
				//*error = ERROR_NONE;
				return temp;
			}
			else {
				LeaveCriticalSection(&csCons);
				Sleep(1);
				//SwitchToThread();
			}

		}
		//else {
		//	//Sleep(1);
		//	SwitchToThread();
		//}
	}
}

// -----------------------------------------------------------------

SWQueueOneToOne::SWQueueOneToOne(int bufSizePower) {
	// buffer initialization
	buf = new UINT64 [bufSizePower];
	mask = (bufSizePower) - 1;
	//bufSize = maxNumItems;
	head = tail = 0;
	prodSema = CreateSemaphore(nullptr, 0, 1, nullptr);
	consSema = CreateSemaphore(nullptr, 0, 1, nullptr);
}

SWQueueOneToOne::~SWQueueOneToOne() {
	delete buf;
	CloseHandle(prodSema);
	CloseHandle(consSema);
}

void SWQueueOneToOne::push(UINT64 item) {
	while (true) {
		int count = 0;
		while (count < SPIN_COUNT) {
			// spins for a bit at full capacity
			register DWORD oldTail = tail;
			register DWORD newTail = (oldTail + 1) & mask;
			if (newTail != head)			// can deposit one item withouth overflowing the queue?
			{
				// empty slots available; add item
				buf[oldTail] = item;
				//_ReadWriteBarrier();
				tail = newTail;
				return;
			}
			count++;
		}

		ReleaseSemaphore(consSema, 1, NULL);
		WaitForSingleObject(prodSema, 1);
	}
}

UINT64 SWQueueOneToOne::pop(void) {
	while (true) {
		register int count = 0;
		while (count < SPIN_COUNT) {
			register int oldHead = head;
			if (oldHead != tail) {
				UINT64 temp = buf[head];
				_ReadWriteBarrier();			// compiler fence; do not update the head before the read is finished
				head = (oldHead + 1) & mask;
				return temp;
			}
			count++;
		}

		ReleaseSemaphore(prodSema, 1, NULL);
		WaitForSingleObject(consSema, 1);
	}
}
