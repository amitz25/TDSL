#include "Index.h"
#include "SafeLock.h"

constexpr ItemType MIN_VAL = -2147483647;

static int NodeCmp(skiplist_node * a, skiplist_node * b, void *)
{
    Node * aa, *bb;
    aa = _get_entry(a, Node, snode);
    bb = _get_entry(b, Node, snode);

    if (aa->key < bb->key) {
        return -1;
    }
    if (aa->key > bb->key) {
        return 1;
    }
    return 0;
}


Index::Index(unsigned int version) : head(MIN_VAL, version)
{
    skiplist_init(&sl, NodeCmp);
    skiplist_insert(&sl, &head.snode);
}

void Index::update(std::vector<IndexOperation> & ops)
{
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
    skiplist_insert(&sl, &n->snode);
    return true;
}

bool Index::remove(Node * n)
{
    skiplist_erase_node(&sl, &n->snode);
    return true;
}

Node * Index::getPrev(const ItemType & k)
{
    Node query(k, 0);
    skiplist_node * cursor = skiplist_find_smaller_or_equal(&sl, &query.snode);
    if (!cursor) {
        throw std::runtime_error("WTF");
    }

    return _get_entry(cursor, Node, snode);
}

long Index::sum()
{
    long sum = 0;
    Node * n = head.next;
    while (n != NULL) {
        sum += n->key;
        n = n->next;
    }

    return sum;
}

long Index::size()
{
    long size = 0;
    Node * n = head.next;
    while (n != NULL) {
        size++;
        n = n->next;
    }

    return size;
}