#pragma once

#include "Utils.h"
#include "Node.h"
#include "SafeLock.h"

class Operation
{
public:
    Operation(Node * next, bool deleted) : next(next), deleted(deleted) {}

    Node * next;
    bool deleted;
};

class WriteSet
{
public:
    void addItem(Node * node, Node * next, bool deleted);

    bool getValue(Node * node, Node *& next, bool * deleted = NULL);

    // TODO: Make sure this doesn't lock/unlock due to copy construction
    bool tryLock(SafeLockList & locks);

    void update(unsigned int newVersion);

private:
    std::unordered_map<Node *, Operation> items;
};
