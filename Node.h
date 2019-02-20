#pragma once

#include "Utils.h"

constexpr ItemType MIN_VAL = -65535;

class Node
{
public:
    Node(const ItemType & k, unsigned int version) :
        key(k), next(NULL), deleted(false), version(version) {}

    virtual ~Node()
    {
    }

    bool isLocked()
    {
        if (lock.try_lock()) {
            lock.unlock();
            return false;
        } else {
            return true;
        }
    }

    ItemType key;
    Node * next;
    bool deleted;
    std::recursive_mutex lock;
    unsigned int version;
};
