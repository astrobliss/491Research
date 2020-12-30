// 491Research.cpp : This file sets up and runs a producer consumer test against a specific ProducerConsumerQueue, outputs metrics to stdout
#include "src/pch.h"
#include "ProducerConsumerQueue.h"
#include "SingleProducerSingleConsumer/SPSCFastForwardQueue.h"
#include "SingleProducerSingleConsumer/SPSCLamportQueue.h"
#include "SingleProducerSingleConsumer/SPSCLamportCacheOptimizedQueue.h"
#include "SingleProducerSingleConsumer/MoodyCamelReaderWriterQueue.h"
#include "SingleProducerSingleConsumer/SynchronizationVariableConstructQueue.h"
#include "SingleProducerSingleConsumer/CASQueue.h"
#include "SingleProducerSingleConsumer/CVQueue.h"
#include "SingleProducerSingleConsumer/SPSCBlockQueue.h"
#include "SingleProducerSingleConsumer/SemaBlockQueue.h"
#include "SingleProducerSingleConsumer/SemaEventQueue.h"

#include "MultiProducerMultiConsumer/MSFTConcurrentQueue.h"
#include "MultiProducerMultiConsumer/SCQueue.h"
#include "MultiProducerMultiConsumer/FacebookQueue.h"
#include "MultiProducerMultiConsumer/MoodyCamelConcurrentQueue.h"
#include "MultiProducerMultiConsumer/SWQueue.h"

#define METRICS_TIMEOUT_MILLISECONDS 2000
#define REQUIRED_NUMBER_OF_CLI_INPUTS 2
#define THRESHOLD_TO_SYNCHRONIZE_LOCAL_WORK (1 << 15)
#define BATCH_WAIT_TIME_MS 10000
#define QUEUE_IMPLIMENTATION SemaEventQueue //SWQueueOneToOne, SPSCLamportQueue, SemaEventQueue
// QUEUE DEFINITION HERE
static QUEUE_IMPLIMENTATION* queue = new QUEUE_IMPLIMENTATION(1 << 12);
static UINT64 totalElementsPushed = 0;
static UINT64 totalElementsPopped = 0;
static HANDLE testStartEvent;

/**
* Consumer Routine for the Producer Consumer Test
* Producers constantly push to the queue and they record how many elements they've pushed to totalElementsPopped
*/
DWORD __stdcall producerRoutine(LPVOID unused) {
	WaitForSingleObject(testStartEvent, INFINITE);
	UINT64 localSent = 0;
	while (true) {
		queue->push(localSent);
		++localSent;
		if (localSent > THRESHOLD_TO_SYNCHRONIZE_LOCAL_WORK) {
			InterlockedAdd64(&totalElementsPushed, localSent);
			localSent = 0;
		}
	}
}

/**
* Consumer Routine for the Producer Consumer Test
* Consumers constantly pop from the queue and they record how many elements they've popped to totalElementsPopped
*/
DWORD __stdcall consumerRoutine(LPVOID unused) {
	WaitForSingleObject(testStartEvent, INFINITE);
	UINT64 localReceived = 0;
	UINT64 currentRecieved;
	while (true) {
		currentRecieved = (UINT64) queue->pop();
		/*if (localReceived != currentRecieved) {
			printf("Expected %lld, Recieved %lld\n", localReceived, currentRecieved);
		}/**/
		++localReceived;
		if (localReceived > THRESHOLD_TO_SYNCHRONIZE_LOCAL_WORK) {
			InterlockedAdd64(&totalElementsPopped, localReceived);
			localReceived = 0;
		}
	}
}

/**
* Prints Push and Pop throughput metrics during Producer Consumer Test
* Wakes every METRICS_TIMEOUT_MILLISECONDS and publishes one metric
*/
DWORD __stdcall printTestMetrics(LPVOID batchModePointer) {
	WaitForSingleObject(testStartEvent, INFINITE);
	UINT64 currentTotalElementsPushed, currentTotalElementsPopped, lastTotalElementsPushed, lastTotalElementsPopped;
	lastTotalElementsPushed = lastTotalElementsPopped = 0;
	clock_t startTime, previousTime, currentTime;
	startTime = previousTime = clock();
	bool batchMode = * static_cast<bool*>(batchModePointer);
	if (batchMode) {
		Sleep(BATCH_WAIT_TIME_MS);
		UINT64 elementsPushed = InterlockedAdd64(&totalElementsPushed, 5);
		UINT64 elementsPopped = InterlockedAdd64(&totalElementsPopped, 5);
		currentTime = clock();
		double totalTimeElapsed = double((currentTime - startTime) / CLOCKS_PER_SEC);
		double averagePushRate = totalElementsPushed / totalTimeElapsed;
		double averagePopRate = totalElementsPopped / totalTimeElapsed;
		printf("%.2f/%.2f", averagePushRate / 1e6, averagePopRate / 1e6);
		return 0;
	}
	while (true) {
		Sleep(METRICS_TIMEOUT_MILLISECONDS); // Consider using SetTimer
        currentTime = clock();
		currentTotalElementsPushed = totalElementsPushed;
		currentTotalElementsPopped = totalElementsPopped;
		double instantTimeElapsed = double((currentTime - previousTime) / CLOCKS_PER_SEC);
		double totalTimeElapsed = double((currentTime - startTime) / CLOCKS_PER_SEC);
		double instantPushRate = (currentTotalElementsPushed - lastTotalElementsPushed)/ instantTimeElapsed;
		double instantPopRate = (currentTotalElementsPopped - lastTotalElementsPopped) / instantTimeElapsed;
		double averagePushRate = currentTotalElementsPushed / totalTimeElapsed;
		double averagePopRate = currentTotalElementsPopped / totalTimeElapsed;
		previousTime = currentTime;
		lastTotalElementsPushed = currentTotalElementsPushed;
		lastTotalElementsPopped = currentTotalElementsPopped;
		printf("Push = %.2f (%.2f) M/s      Pop = %.2f (%.2f) M/s\n", averagePushRate /1e6, instantPushRate /1e6, averagePopRate / 1e6, instantPopRate / 1e6);
	}
}


/**
* Main method, takes two command line inputs NumberOfProducers and NumberOfConsumers both as ints
* Initalizes and starts a Producer Consumer Test with the specified number of producers and consumers which will run until the process is stopped
*/
int main(int argc, char** argv) {
	/*
	DWORD minimumResolution, currentResolution, maximumResolution;
	NtQueryTimerResolution(&maximumResolution, &minimumResolution, &currentResolution);
	NtSetTimerResolution(10000, TRUE, &currentResolution);
	printf("current resolution: %.2f\nminimum resolution: %.2f\n maximum resolution: %.2f\n", currentResolution/1e4, minimumResolution/1e4, maximumResolution/1e4);/**/
	// Read Command Line Input: Errors have exit code 1
	int numberOfProducers, numberOfConsumers;
	bool batchMode;
	int presentNumberOfCliInputs = argc - 1;
	if (presentNumberOfCliInputs < REQUIRED_NUMBER_OF_CLI_INPUTS) {
		printf("Incorrect number of CLI Inputs, required at least %d, recieved %d\n", REQUIRED_NUMBER_OF_CLI_INPUTS, presentNumberOfCliInputs);
		exit(1);
	}
	numberOfProducers = strtol(argv[1], NULL, 10);
	numberOfConsumers = strtol(argv[2], NULL, 10);
	batchMode = (presentNumberOfCliInputs > REQUIRED_NUMBER_OF_CLI_INPUTS);

	// Initalize Test Resources: Errors have exit code 2
	if (!batchMode) {
		printf("Initalizing Producer Consumer Test with %d Producers and %d Consumers using the %s Queue Implementation\n", numberOfProducers, numberOfConsumers, "queue name");
	}
	HANDLE* producers = new HANDLE[numberOfProducers];
	HANDLE* consumers = new HANDLE[numberOfConsumers];
	HANDLE metricsThread;
	testStartEvent = CreateEvent(NULL, true, false, NULL);
	if (testStartEvent == NULL) {
		printf("Error creating test start event\n");
		exit(2);
	}
	for (int i = 0; i < numberOfProducers; i++) {
		producers[i] = CreateThread(nullptr, 0, producerRoutine, nullptr, 0, nullptr);
		SetThreadAffinityMask(producers[i], 1);
		if (producers[i] == nullptr) {
			printf("Error Creating the %dth Producer Thread\n", i);
			exit(2);
		}
		bool setPrioritySuccessful = SetThreadPriority(producers[i], THREAD_PRIORITY_IDLE);
		if (!setPrioritySuccessful) {
			printf("Error Setting the %dth Producer Thread Priority\n", i);
			exit(2);
		}
	}
	for (int i = 0; i < numberOfConsumers; i++) {
		consumers[i] = CreateThread(nullptr, 0, consumerRoutine, nullptr, 0, nullptr);
		SetThreadAffinityMask(consumers[i], 2);
		if (consumers[i] == nullptr) {
			printf("Error Creating the %dth Consumer Thread\n", i);
			exit(2);
		}
		bool setPrioritySuccessful = SetThreadPriority(consumers[i], THREAD_PRIORITY_IDLE);
		if (!setPrioritySuccessful) {
			printf("Error Setting the %dth Producer Thread Priority\n", i);
			exit(2);
		}
	}
	metricsThread = CreateThread(nullptr, 0, printTestMetrics, &batchMode, 0, nullptr);
	if (metricsThread == nullptr) {
		printf("Error Creating Metrics Thread\n");
		exit(2);
	}
	SetThreadPriority(metricsThread, THREAD_PRIORITY_HIGHEST);
	SetThreadAffinityMask(metricsThread, 4);

	// Start test: Errors have exit code 3
	bool isTestSuccessfullyStarted = SetEvent(testStartEvent);
	if (!isTestSuccessfullyStarted) {
		printf("Error starting Producer Consumer Test\n");
		exit(3);
	}

	//Test is running kill process to stop
	if (!batchMode) {
		printf("Test is started...\n");
	}
	WaitForSingleObject(metricsThread, INFINITE);

	// no more metrics
	ExitProcess(0);
}
