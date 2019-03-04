#pragma once

#include "Node.h"
#include "Utils.h"
#include "skiplist/skiplist.h"

enum OperationType
{
    REMOVE,
    INSERT,
    CONTAINS
};

class IndexOperation
{
public:
    IndexOperation(Node * node, OperationType op) : node(node), op(op) {}

    Node * node;
    OperationType op;
};

class Index
{
public:
    Index(unsigned int version);

    void update(std::vector<IndexOperation> & ops);

    bool insert(Node * node);

    bool remove(Node * node);


    Node * getPrev(const ItemType & k);

    // These methods are purely for test-purposes and are not meant to be used by TDSs.
    bool insert(const ItemType & k);
    bool remove(const ItemType & k);
    bool contains(const ItemType & k);
    Node * getNode(const ItemType & k);
    ItemType sum();

private:
    Node head;
    skiplist_raw sl;
};
