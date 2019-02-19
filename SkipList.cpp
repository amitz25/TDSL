#include "SkipList.h"

void SkipList::TXBegin(SkipListTransaction & transaction)
{
    transaction.readVersion = gvc.read();
}

bool SkipList::contains(ItemType & k, SkipListTransaction & transaction)
{
    Node * pred = NULL, *succ = NULL;
    traverseTo(k, transaction, pred, succ);

    if (succ != NULL && succ->key == k) {
        return true;
    } else {
        return false;
    }
}

bool SkipList::insert(ItemType & k, SkipListTransaction & transaction)
{
    Node * pred = NULL, *succ = NULL;
    traverseTo(k, transaction, pred, succ);

    if (succ != NULL && succ->key == k) {
        return false;
    }

    Node * newNode = new Node(k);
    newNode->next = succ;

    transaction.writeSet.addItem(pred, newNode, false);
    transaction.writeSet.addItem(newNode, NULL, false);
    return true;
}

bool SkipList::remove(ItemType & k, SkipListTransaction & transaction)
{
    Node * pred = NULL, *succ = NULL;
    traverseTo(k, transaction, pred, succ);

    if (succ == NULL || succ->key != k) {
        return false;
    }

    transaction.readSet.push_back(succ);

    transaction.writeSet.addItem(pred, succ->next, false);
    transaction.writeSet.addItem(succ, NULL, true);
    return true;
}

void SkipList::TXCommit(SkipListTransaction & transaction)
{
    // TODO
}

void SkipList::traverseTo(ItemType & k, SkipListTransaction & transaction,
                          Node *& pred, Node *& succ)
{

}