#pragma once

#include "Utils.h"

class SafeLock
{
public:
    SafeLock(Mutex & m) : lock(m)
    {
        lock.lock();
    }

    virtual ~SafeLock()
    {
        lock.unlock();
    }

private:
    Mutex & lock;
};

class SafeLockList
{
public:
    SafeLockList() {}

    virtual ~SafeLockList()
    {
        // Unlock in reverse order
        for (auto it = locks.rbegin(); it != locks.rend(); ++it) {
            (*it)->unlock();
        }
    }

    void add(Mutex & lock)
    {
        locks.push_back(&lock);
    }

private:
    std::vector<Mutex *> locks;
};