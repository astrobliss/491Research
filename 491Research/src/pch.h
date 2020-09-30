// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#include <windows.h>	/* Used for low level Windows Synchronization methods */
#include <cstdio>		/* Used for rich efficent printout */
#include <math.h>		/* Used for basic math functions */
#include <stdexcept>    /* Used for basic exception types */
#include <time.h>       /* Used to measure execution time */
#include <typeinfo>     /* Used to output ProducerConsumerQueue Implementation name */

#define UINT64 __int64  /* Current 64-bit integer implimentation */

#endif //PCH_H
