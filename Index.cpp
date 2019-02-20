#include "Index.h"
#include "SafeLock.h"

void Index::update(std::vector<IndexOperation> & ops)
{
    SafeLock sl(lock);
    for (auto & op : ops) {
        if (op.op == OperationType::REMOVE) {
            if (!remove(op.node)) {
                throw std::runtime_error("Failed during Index update!");
            }
        } else {
            if (!insert(op.node)) {
                throw std::runtime_error("Failed during Index update!");
            }
        }
    }
}

bool Index::insert(Node * n)
{
    // TODO: This function does nothing since we've already updated the nodes. When we use
    //       a real skiplist for efficiency we should change this function
    return true;
}

bool Index::remove(Node * n)
{
    // TODO: This function does nothing since we've already updated the nodes. When we use
    //       a real skiplist for efficiency we should change this function
    return true;
}

Node * Index::getPrev(const ItemType & k)
{
    Node * n = head;
    while (n->next != NULL && n->next->key < k) {
        n = n->next;
    }

    return n;
}


bool Index::remove(const ItemType & k)
{
    Node * n = getPrev(k);

    if (n->next == NULL || n->next->key != k) {
        return false;
    }

    Node * todelete = n->next;
    n->next = todelete->next;
    todelete->deleted = true;
    return true;
}

bool Index::insert(const ItemType & k)
{
    Node * n = getPrev(k);

    if (n->next != NULL && n->next->key == k) {
        return false;
    }

    Node * newNode = new Node(k, 0);

    newNode->next = n->next;
    n->next = newNode;
    return true;
}

bool Index::contains(const ItemType & k)
{
    Node * n = getPrev(k);
    return n->next != NULL && n->next->key == k;
}

ItemType Index::sum()
{
    ItemType sum = 0;
    Node * n = head->next;
    while (n != NULL) {
        sum += n->key;
        n = n->next;
    }

    return sum;
}

Node * Index::getNode(const ItemType & k)
{
    Node * n = getPrev(k);
    return n->next;
}