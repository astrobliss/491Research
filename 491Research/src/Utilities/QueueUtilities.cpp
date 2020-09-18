#pragma once
#include "src/pch.h"

/**
* Efficently finds the element to the right of an elemnt in a buffer of a certain size, wraps around to zero if no elements to the right
*/
inline UINT64 nextBufferElement(UINT64 currentElement, UINT64 bufferSize) {
    if (currentElement + 1 == bufferSize) {
        return 0;
    }
    else {
        return currentElement + 1;
    }
}
inline int nextBufferElement(int currentElement, int bufferSize) {
    if (currentElement + 1 == bufferSize) {
        return 0;
    }
    else {
        return currentElement + 1;
    }
}