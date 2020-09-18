// 491Research.cpp : This file sets up and runs a producer consumer test against a specific ProducerConsumerQueue, outputs metrics to stdout
#include "src/pch.h"
#include "ProducerConsumerQueue.h"
#include "SingleProducerSingleConsumer/SPSCFastForwardQueue.h"
#include "SingleProducerSingleConsumer/SPSCLamportQueue.h"
#include "SingleProducerSingleConsumer/SPSCLamportCacheOptimizedQueue.h"
#define METRICS_TIMEOUT_MILLISECONDS 2000
#define REQUIRED_NUMBER_OF_CLI_INPUTS 2
#define THRESHOLD_TO_SYNCHRONIZE_LOCAL_WORK 1 << 15
// QUEUE DEFINITION HERE
static ProducerConsumerQueue* queue = new SPSCFastForwardQueue(100000);
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
	const UINT64 threshold = 1 << 3;
	while (true) {
		queue->pop();
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
DWORD __stdcall printTestMetrics(LPVOID unused) {
	WaitForSingleObject(testStartEvent, INFINITE);
	UINT64 currentTotalElementsPushed, currentTotalElementsPopped, lastTotalElementsPushed, lastTotalElementsPopped;
	lastTotalElementsPushed = lastTotalElementsPopped = 0;
	clock_t previousTime, currentTime;
	previousTime = clock();
	while (true) {
		Sleep(METRICS_TIMEOUT_MILLISECONDS); // Consider using SetTimer
        currentTime = clock();
		currentTotalElementsPushed = totalElementsPushed;
		currentTotalElementsPopped = totalElementsPopped;
		double timeElapsed = double((currentTime - previousTime) / CLOCKS_PER_SEC);
		double pushRate = (currentTotalElementsPushed - lastTotalElementsPushed)/timeElapsed;
		double popRate = (currentTotalElementsPopped - lastTotalElementsPopped) /timeElapsed;
		printf("Push Rate %.2fM/sec\tPop Rate %.2fM/sec\n", pushRate/1e6, popRate/1e6);
		lastTotalElementsPushed = currentTotalElementsPushed;
		lastTotalElementsPopped = currentTotalElementsPopped;
	}
}

/**
* Main method, takes two command line inputs NumberOfProducers and NumberOfConsumers both as ints
* Initalizes and starts a Producer Consumer Test with the specified number of producers and consumers which will run until the process is stopped
*/
int main(int argc, char** argv) {
	// Read Command Line Input: Errors have exit code 1
	int numberOfProducers, numberOfConsumers;
	int presentNumberOfCliInputs = argc - 1;
	if (presentNumberOfCliInputs != REQUIRED_NUMBER_OF_CLI_INPUTS) {
		printf("Incorrect number of CLI Inputs, expected %d, recieved %d\n", REQUIRED_NUMBER_OF_CLI_INPUTS, presentNumberOfCliInputs);
		exit(1);
	}
	numberOfProducers = strtol(argv[1], NULL, 10);
	numberOfConsumers = strtol(argv[2], NULL, 10);

	// Initalize Test Resources: Errors have exit code 2
	printf("Initalizing Producer Consumer Test with %d Producers and %d Consumers using the %s Queue Implementation\n", numberOfProducers, numberOfConsumers, typeid(*queue).name());
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
		if (producers[i] == nullptr) {
			printf("Error Creating the %dth Producer Thread\n", i);
			exit(2);
		}
	}
	for (int i = 0; i < numberOfConsumers; i++) {
		consumers[i] = CreateThread(nullptr, 0, consumerRoutine, nullptr, 0, nullptr);
		if (consumers[i] == nullptr) {
			printf("Error Creating the %dth Consumer Thread\n", i);
			exit(2);
		}
	}
	metricsThread = CreateThread(nullptr, 0, printTestMetrics, nullptr, 0, nullptr);
	if (metricsThread == nullptr) {
		printf("Error Creating Metrics Thread\n");
		exit(2);
	}

	// Start test: Errors have exit code 3
	bool isTestSuccessfullyStarted = SetEvent(testStartEvent);
	if (!isTestSuccessfullyStarted) {
		printf("Error starting Producer Consumer Test\n");
		exit(3);
	}

	//Test is running kill process to stop
	printf("Test is started...\n");
	WaitForSingleObject(metricsThread, INFINITE);
}
