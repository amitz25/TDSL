#pragma once

#include "Utils.h"

class Node
{
public:
    Node(ItemType & k) : key(k), next(NULL) {}

    virtual ~Node()
    {
        if (next) {
            delete next;
        }
    }

    ItemType key;
    Node * next;
    std::mutex lock;
};
