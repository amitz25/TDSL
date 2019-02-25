#pragma once

#include "Utils.h"

// TODO: isLocked() is inefficient
class Mutex
{
public:
    Mutex() {}

    virtual ~Mutex() {}

    void lock()
    {
        mutex.lock();
    }

    void unlock()
    {
        mutex.unlock();
    }

    bool isLocked()
    {
        if (mutex.try_lock()) {
            mutex.unlock();
            return false;
        } else {
            return true;
        }
    }

    bool tryLock()
    {
        return mutex.try_lock();
    }

private:
    std::recursive_mutex mutex;
};
