#pragma once

#include "Utils.h"

class SafeLock
{
public:
    SafeLock(std::recursive_mutex & m) : lock(m)
    {
        lock.lock();
    }

    virtual ~SafeLock()
    {
        lock.unlock();
    }

private:
    std::recursive_mutex & lock;
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

    void add(std::recursive_mutex & lock)
    {
        locks.push_back(&lock);
    }

private:
    std::vector<std::recursive_mutex *> locks;
};