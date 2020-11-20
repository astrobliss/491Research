#include "src/pch.h"
#include "GlobalSCQueue.h"
#include "src/SingleProducerSingleConsumer/SemaEventQueue.h"

/*
void GlobalSCQueue::initalizeProducer(SemaEventQueue*& prodCurQueue) {
    EnterCriticalSection(&processInitilization);
    ++numProducers;
    producerQueueAssignment.push_back(std::make_pair(0, &prodExtraQueues));
    SemaEventQueue* newQueue = nullptr;
    if (queues.size() < std::max(numProducers, numConsumers)) {
        newQueue = new SemaEventQueue(SIZE_PER_QUEUE);
        queues.push_back(newQueue);
        prodExtraQueues.insert(newQueue);
        ++producerQueueAssignment.back().first;
    }
    balanceQueueAssignment(newQueue, newQueue);
    prodExtraQueues.remove(prodCurQueue);
    LeaveCriticalSection(&processInitilization);
    producerInitalized = true;
}

void GlobalSCQueue::initalizeConsumer(SemaEventQueue*& consCurQueue) {
    EnterCriticalSection(&processInitilization);
    ++numConsumers;
    consumerQueueAssignment.push_back(std::make_pair(0, &consExtraQueues));
    SemaEventQueue* newQueue = nullptr;
    if (queues.size() < std::max(numProducers, numConsumers)) {
        newQueue = new SemaEventQueue(SIZE_PER_QUEUE);
        queues.push_back(newQueue);
        consExtraQueues.insert(newQueue);
        ++consumerQueueAssignment.back().first;
    }
    balanceQueueAssignment(newQueue, newQueue);
    consExtraQueues.remove(consCurQueue);
    LeaveCriticalSection(&processInitilization);
    consumerInitalized = true;
}

void GlobalSCQueue::balanceQueueAssignment(bool hasNewQueue, SemaEventQueue* newQueue) {
    std::vector<SemaEventQueue*> freedQueues;
    if (hasNewQueue) {
        freedQueues.push_back(newQueue);
    }
    // balance Producers
    if (numProducers > 0) {
        if (numProducers == 1 && producerQueueAssignment.front().first == 0) {
            producerQueueAssignment.front().first = queues.size();
            for (SemaEventQueue* queue : queues) {
                producerQueueAssignment.front().second->insert(queue);
            }
        }
        else {
            int queuesPerProducer = queues.size() / numProducers;
            int extraProducerQueues = queues.size() % numProducers;
            // the first extraProducerQueues many producers should have 1+queuesPerProducer many queues
            // therefore ith producer should have (queuesPerProducer + ((i<extraProducerQueues)?1:0)
            for (int i = 0; i < numProducers; ++i) {
                int correctNumQueues = queuesPerProducer + ((i < extraProducerQueues) ? 1 : 0);
                while (producerQueueAssignment[i].first > correctNumQueues) {
                    SemaEventQueue* freedQueue;
                    producerQueueAssignment[i].second->remove(freedQueue);
                    freedQueues.push_back(freedQueue);
                    --producerQueueAssignment[i].first;
                }
            }
            for (int i = 0; i < numProducers; ++i) {
                int correctNumQueues = queuesPerProducer + ((i < extraProducerQueues) ? 1 : 0);
                while (producerQueueAssignment[i].first < correctNumQueues) {
                    producerQueueAssignment[i].second->insert(freedQueues.back());
                    ++producerQueueAssignment[i].first;
                    freedQueues.pop_back();
                }
            }
        }
    }

    // balance Consumers
    if (numConsumers > 0) {
        if (numConsumers == 1 && consumerQueueAssignment.front().first == 0) {
            consumerQueueAssignment.front().first = queues.size();
            for (SemaEventQueue* queue : queues) {
                consumerQueueAssignment.front().second->insert(queue);
            }
        }
        else {
            int queuesPerConsumer = queues.size() / numConsumers;
            int extraConsumerQueues = queues.size() % numConsumers;
            for (int i = 0; i < numConsumers; ++i) {
                int correctNumQueues = queuesPerConsumer + ((i < extraConsumerQueues) ? 1 : 0);
                while (consumerQueueAssignment[i].first > correctNumQueues) {
                    SemaEventQueue* freedQueue;
                    consumerQueueAssignment[i].second->remove(freedQueue);
                    freedQueues.push_back(freedQueue);
                    --consumerQueueAssignment[i].first;
                }
            }
            for (int i = 0; i < numConsumers; ++i) {
                int correctNumQueues = queuesPerConsumer + ((i < extraConsumerQueues) ? 1 : 0);
                while (consumerQueueAssignment[i].first < correctNumQueues) {
                    consumerQueueAssignment[i].second->insert(freedQueues.back());
                    ++consumerQueueAssignment[i].first;
                    freedQueues.pop_back();
                }
            }
        }
    }
}

SCQueue::SCQueue(int maxCapacity) {
    InitializeCriticalSection(&processInitilization);
}

SCQueue::~SCQueue() {
    for (SemaEventQueue* queue : queues) {
        delete queue;
    }
}

void GlobalSCQueue::push(UINT64 newElement) {
    thread_local SemaEventQueue* curQueue;
    thread_local ConcurrentLookupSet<SemaEventQueue> extraQueues;
    thread_local bool producerInitalized = false;
    if (!producerInitalized) {
        initalizeProducer(curQueue, extraQueues, producerInitalized);
    }
    while (true) {
        for (int i = 0; i < attemptsUntilSleep;) {
            // try to push to the current queue
            if (curQueue->tryPush(newElement)) {
                return;
            }

            // try to use a different queue
            if (!extraQueues.try_swap(curQueue) && (++i < attemptsUntilSleep)) {
                break;
            }
        }
        curQueue->prodSync();
    }
}

UINT64 GlobalSCQueue::pop() {
    thread_local SemaEventQueue* curQueue;
    thread_local ConcurrentLookupSet<SemaEventQueue> extraQueues;
    thread_local bool consumerInitalized = false;
    if (!consumerInitalized) [[unlikely]] {
        initalizeConsumer(curQueue, extraQueues, consumerInitalized);
    }
        while (true) {
            for (int i = 0; i < attemptsUntilSleep;) {
                // try to push to the current queue
                UINT64 output;
                if (curQueue->tryPop(output)) {
                    return output;
                }

                // try to use a different queue
                if (!extraQueues.try_swap(curQueue) && (++i < attemptsUntilSleep)) {
                    break;
                }
            }
            curQueue->consSync();
        }
}

UINT64 GlobalSCQueue::getCapacity() {
    return SIZE_PER_QUEUE * queues.size();
}
*/