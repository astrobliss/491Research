#pragma once

// Set that where elements can be inserted and removed
// Linear complexity in terms of all elements
template<typename T>
class ConcurrentLookupSet {
private:
    // Use Read/Write fence instead of mutex to increase preformance
    CRITICAL_SECTION mutex;
    std::vector<std::pair<bool,T*>> set;
    int curPosition = 0;
public:
    // Constructs an empty ConcurrentLookupSet
    ConcurrentLookupSet() {
        InitializeCriticalSection(&mutex);
    }

    // Inserts an elment into the set, if the element already exists, then this does nothing
    void insert(T* element) {
        EnterCriticalSection(&mutex);
        for (int i = 0; i < set.size(); ++i) {
            if (set[i].second == element) {
                set[i].first = true;
                LeaveCriticalSection(&mutex);
                return;
            }
        }
        set.push_back(std::make_pair(true, element));
        LeaveCriticalSection(&mutex);
    }

    // Attempts to removes an element from the set
    // element will hold a pointer to the elment removed from the set if the remove was successful
    // return true iff the remove was successful
    bool remove(T*& element) {
        EnterCriticalSection(&mutex);
        for (int i = curPosition; i < set.size(); ++i) {
            if (set[i].first == true) {
                set[i].first = false;
                curPosition = i;
                LeaveCriticalSection(&mutex);
                element = set[i].second;
                return true;
            }
        }
        for (int i = 0; i < curPosition; ++i) {
            if (set[i].first == true) {
                set[i].first = false;
                curPosition = i;
                LeaveCriticalSection(&mutex);
                element = set[i].second;
                return true;
            }
        }
        LeaveCriticalSection(&mutex);
        return false;
    }

    // Swaps element with a member of the set, if the set is empty or element is already in the set, the swap fails
    // element: On successful swap, element will be inserted into the set and the value element will then point to a set value which is now removed
    // returns true iff the swap was successful
    bool try_swap(T*& element) {
        EnterCriticalSection(&mutex);
        T* removedElement;
        if (remove(removedElement)) {
            insert(element);
            element = removedElement;
            LeaveCriticalSection(&mutex);
            return true;
        }
        LeaveCriticalSection(&mutex);
        return false;
    }
};
