#pragma once

#include "Utils.h"
#include "WriteSet.h"
#include "Node.h"
#include "GVC.h"
#include "Index.h"


class SkipListTransaction
{
public:
    SkipListTransaction() {}

    virtual ~SkipListTransaction() {}

    unsigned int readVersion;
    unsigned int writeVersion;
    std::vector<Node *> readSet;
    WriteSet writeSet;
    std::vector<IndexOperation> indexTodo;
};

class SkipList
{
public:
    SkipList() : index(gvc.read()) {}

    virtual ~SkipList() {}

    void TXBegin(SkipListTransaction & transaction);

    bool contains(const ItemType & k, SkipListTransaction & transaction);

    bool insert(const ItemType & k, SkipListTransaction & transaction);

    bool remove(const ItemType & k, SkipListTransaction & transaction);

    Node * getValidatedValue(SkipListTransaction & transaction, Node * node,
                             bool * outDeleted = NULL);

    bool validateReadSet(SkipListTransaction & transaction);

    void TXCommit(SkipListTransaction & transaction);

    void traverseTo(const ItemType & k, SkipListTransaction & transaction,
                    Node *& pred, Node *& succ);

    GVC gvc;
    Index index;
};
