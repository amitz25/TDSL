#pragma once

#include "Utils.h"

class Mutex
{
public:
    Mutex() : locked(false) {}

    virtual ~Mutex() {}

    void lock()
    {
        locked = true;
        mutex.lock();
    }

    void unlock()
    {
        mutex.unlock();
        locked = false;
    }

    bool isLocked()
    {
        return locked;
    }

    bool tryLock()
    {
        return mutex.try_lock();
    }

private:
    std::recursive_mutex mutex;
    bool locked;
};
