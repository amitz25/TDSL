#pragma once

#include "Utils.h"
#include "Node.h"

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

private:
    std::unordered_map<Node *, Operation> items;
};
