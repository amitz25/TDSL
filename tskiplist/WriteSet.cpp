#include "WriteSet.h"

void WriteSet::addItem(Node * node, Node * next, bool deleted)
{
    const auto it = items.find(node);
    if (it == items.end()) {
        items.insert(std::pair<Node *, Operation>(node, Operation(next, deleted)));
    } else {
        if (next) {
            it->second.next = next;
        }
        if (deleted) {
            it->second.deleted = deleted;
        }
    }
}

bool WriteSet::getValue(Node * node, Node *& next, bool * deleted)
{
    auto it = items.find(node);
    if (it == items.end()) {
        return false;
    }

    if (deleted) {
        *deleted = it->second.deleted;
    }

    if (it->second.next != NULL) {
        next = it->second.next;
    } else {
        next = node->next;
    }

    return true;
}

bool WriteSet::tryLock(SafeLockList & locks)
{
    for (auto & it : items) {
        Node * node = it.first;
        if (node->lock.tryLock()) {
            locks.add(node->lock);
        } else {
            return false;
        }
    }
    return true;
}

void WriteSet::update(unsigned int newVersion)
{
    for (auto & it : items) {
        Node * n = it.first;
        Operation & op = it.second;

        if (op.deleted) {
            n->deleted = op.deleted;
        }

        if (op.next) {
            n->next = op.next;
        }

        n->version = newVersion;
    }
}