#pragma once
#include "src/pch.h"

/**
* Bounded Queue Interface containing all methods of interest for the 2020-2021 491H Queue throughput/latency tradeoff investigation
* Implimentations should allow simultanous access across threads and block threads until they recieve or push an element
* Consistency guarntees across threads discussed in full paper, release is TBD
*/
class ProducerConsumerQueue
{
public:
	/**
	* Pushes one element into the queue to later be accessed by a pop, note this operation is not guaranteed to be FIFO or Lineariazable
	*/
	virtual void push(UINT64) = 0;

	/**
	* Pops one element from the queue which was previously pushed, note this operation is not guaranteed to be FIFO or Lineariazable
	*/
	virtual UINT64 pop() = 0;

	/**
	* Returns the size the Queue is bounded under
	*/
	virtual UINT64 getCapacity() = 0;
};

