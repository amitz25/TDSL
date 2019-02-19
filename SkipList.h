#pragma once

#include "Utils.h"
#include "WriteSet.h"
#include "Node.h"
#include "GVC.h"

class SkipListTransaction
{
public:
    SkipListTransaction() {}

    // TODO: Should this free all locks etc.?
    virtual ~SkipListTransaction() {}

    unsigned int readVersion;
    std::vector<Node *> readSet;
    WriteSet writeSet;
};

class SkipList
{
public:
    SkipList() : head(NULL) {}

    virtual ~SkipList() {}

    void TXBegin(SkipListTransaction & transaction);

    bool contains(ItemType & k, SkipListTransaction & transaction);

    bool insert(ItemType & k, SkipListTransaction & transaction);

    bool remove(ItemType & k, SkipListTransaction & transaction);

    void TXCommit(SkipListTransaction & transaction);

private:
    void traverseTo(ItemType & k, SkipListTransaction & transaction,
                    Node *& pred, Node *& succ);

    Node * head;
    GVC gvc;
};
