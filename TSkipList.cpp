#include "TSkipList.h"

#include "SafeLock.h"

void SkipList::TXBegin(SkipListTransaction & transaction)
{
    transaction.readVersion = gvc.read();
}

bool SkipList::contains(const ItemType & k, SkipListTransaction & transaction)
{
    Node * pred = NULL, *succ = NULL;
    traverseTo(k, transaction, pred, succ);

    if (succ != NULL && succ->key == k) {
        return true;
    } else {
        return false;
    }
}

bool SkipList::insert(const ItemType & k, SkipListTransaction & transaction)
{
    Node * pred = NULL, *succ = NULL;
    traverseTo(k, transaction, pred, succ);

    if (succ != NULL && succ->key == k) {
        return false;
    }

    Node * newNode = new Node(k, transaction.readVersion);
    newNode->next = succ;

    transaction.writeSet.addItem(pred, newNode, false);
    transaction.writeSet.addItem(newNode, NULL, false);
    transaction.indexTodo.push_back(IndexOperation(newNode, OperationType::INSERT));
    return true;
}

bool SkipList::remove(const ItemType & k, SkipListTransaction & transaction)
{
    Node * pred = NULL, *succ = NULL;
    traverseTo(k, transaction, pred, succ);

    if (succ == NULL || succ->key != k) {
        return false;
    }

    transaction.readSet.push_back(succ);

    transaction.writeSet.addItem(pred, getValidatedValue(transaction, succ), false);
    transaction.writeSet.addItem(succ, NULL, true);
    transaction.indexTodo.push_back(IndexOperation(succ, OperationType::REMOVE));
    return true;
}

Node * SkipList::getValidatedValue(SkipListTransaction & transaction,
                                   Node * node, bool * outDeleted)
{
    Node * res = NULL;
    if (node->isLocked()) {
        throw AbortTransactionException();
    }

    if (!transaction.writeSet.getValue(node, res, outDeleted)) {
        res = node->next;

        if (outDeleted) {
            *outDeleted = node->deleted;
        }
    }

    if (node->version > transaction.readVersion) {
        throw AbortTransactionException();
    }

    if (node->isLocked()) {
        throw AbortTransactionException();
    }

    return res;
}

bool SkipList::validateReadSet(SkipListTransaction & transaction)
{
    for (auto n : transaction.readSet) {
        if (!getValidatedValue(transaction, n)) {
            return false;
        }
    }
    return true;
}

void SkipList::TXCommit(SkipListTransaction & transaction)
{
    {
        SafeLockList locks;
        if (!transaction.writeSet.tryLock(locks)) {
            throw AbortTransactionException();
        }

        if (!validateReadSet(transaction)) {
            throw AbortTransactionException();
        }

        transaction.writeVersion = gvc.addAndFetch();
        transaction.writeSet.update(transaction.writeVersion);
    }
    index.update(transaction.indexTodo);
}

void SkipList::traverseTo(const ItemType & k, SkipListTransaction & transaction,
                          Node *& pred, Node *& succ)
{
    Node * startNode = index.getPrev(k);
    bool deleted = false;
    succ = getValidatedValue(transaction, startNode, &deleted);
    while (startNode->isLocked() || deleted) {
        startNode = index.getPrev(startNode->key);
        succ = getValidatedValue(transaction, startNode, &deleted);
    }

    pred = startNode;
    deleted = false;
    while (succ != NULL && (succ->key < k || deleted)) {
        pred = succ;
        succ = getValidatedValue(transaction, pred, &deleted);
    }
    transaction.readSet.push_back(pred);
}