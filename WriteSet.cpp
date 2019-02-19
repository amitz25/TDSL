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