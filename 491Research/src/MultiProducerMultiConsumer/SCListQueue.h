#pragma once
#include "pch.h"
#include "src/ConcurrentLookupSet.h"
#include "src/SingleProducerSingleConsumer/SemaEventQueue.h"


class SCListQueue {
private:
    const UINT64 SIZE_PER_QUEUE = (1 << 15);
    const UINT64 attemptsUntilSleep = 100;
    CRITICAL_SECTION processInitilization;
    std::vector<SemaEventQueue*> queues;
    std::vector<std::pair<int, ConcurrentLookupSet<SemaEventQueue>*>> producerQueueAssignment, consumerQueueAssignment;
    UINT64 numProducers = 0;
    UINT64 numConsumers = 0;
    void initalizeProducer(SemaEventQueue*& prodCurQueue, ConcurrentLookupSet<SemaEventQueue>& prodExtraQueues, bool& producerInitalized);
    void initalizeConsumer(SemaEventQueue*& consCurQueue, ConcurrentLookupSet<SemaEventQueue>& consExtraQueues, bool& consumerInitalized);
    void balanceQueueAssignment(bool hasNewQueue, SemaEventQueue* newQueue);

    //DWORD __stdcall producerHelperRoutine(LPVOID unused);
    //DWORD __stdcall consumerHelperRoutine(LPVOID unused);
public:
    /**
    * Constructs a Sequentially Consistent Queue ProducerConsumerQueue
    */
    SCListQueue(int ignored);
    ~SCListQueue();
    void push(UINT64);
    UINT64 pop();
    UINT64 getCapacity();
};

