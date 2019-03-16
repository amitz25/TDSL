/******************************************************************************
 * skip_cas.c
 * 
 * Skip lists, allowing concurrent update by use of CAS primitives. 
 * 
 * Copyright (c) 2001-2003, K A Fraser
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// NOTE: this was originally c code, so we pass a pointer to our transskip, l, since there is no transskip class 

#define __SET_IMPLEMENTATION__

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
extern "C"
{
#include "common/fraser/portable_defns.h"
#include "common/fraser/ptst.h"
}
#include "transskip.h"
#include "../tskiplist/TSkipList.h"

#define SET_MARK(_p)    ((node_t *)(((uintptr_t)(_p)) | 1))
#define CLR_MARKD(_p)    ((NodeDesc *)(((uintptr_t)(_p)) & ~1))
#define IS_MARKED(_p)     (((uintptr_t)(_p)) & 1)

struct HelpStack
{
    void Init()
    {
        index = 0;
    }

    void Push(Desc* desc)
    {
        ASSERT(index < 255, "index out of range");

        helps[index++] = desc;
    }

    void Pop()
    {
        ASSERT(index > 0, "nothing to pop");

        index--;
    }

    bool Contain(Desc* desc)
    {
        for(uint8_t i = 0; i < index; i++)
        {
            if(helps[i] == desc)
            {
                return true;
            }
        }

        return false;
    }

    Desc* helps[256];
    uint8_t index;
};

static __thread HelpStack helpStack;

enum OpStatus
{
    LIVE = 0,
    COMMITTED,
    ABORTED
};

enum OpType
{
    O_FIND = 0,
    O_INSERT,
    O_DELETE
};

static int gc_id[NUM_LEVELS];

static uint32_t g_count_commit = 0;
static uint32_t g_count_abort = 0;
static uint32_t g_count_fake_abort = 0;

/*
 * PRIVATE FUNCTIONS
 */

static bool help_ops(trans_skip* l, Desc* desc, uint8_t opid);

static inline bool FinishPendingTxn(trans_skip* l, NodeDesc* nodeDesc, Desc* desc)
{
    // The node accessed by the operations in same transaction is always active 
    if(nodeDesc->desc == desc)
    {
        return true;
    }

    if(nodeDesc->desc->status == LIVE)
    {
        help_ops(l, nodeDesc->desc, nodeDesc->opid + 1);
    }

    return true;
}

static inline bool IsNodeActive(NodeDesc* nodeDesc)
{
    return nodeDesc->desc->status == COMMITTED;
}

static inline bool IsKeyExist(NodeDesc* nodeDesc)
{
    bool isNodeActive = IsNodeActive(nodeDesc);
    uint8_t opType = nodeDesc->desc->ops[nodeDesc->opid].type;

    return (opType == O_FIND) || (isNodeActive && opType == O_INSERT) || (!isNodeActive && opType == O_DELETE);
}

static inline bool IsSameOperation(NodeDesc* nodeDesc1, NodeDesc* nodeDesc2)
{
    return nodeDesc1->desc == nodeDesc2->desc && nodeDesc1->opid == nodeDesc2->opid;
}


/*
 * Random level generator. Drop-off rate is 0.5 per level.
 * Returns value 1 <= level <= NUM_LEVELS.
 */
static int get_level(ptst_t *ptst)
{
    unsigned long r = rand_next(ptst);
    int l = 1;
    r = (r >> 4) & ((1 << (NUM_LEVELS-1)) - 1);
    while ( (r & 1) ) { l++; r >>= 1; }
    return(l);
}


/*
 * Allocate a new node, and initialise its @level field.
 * NB. Initialisation will eventually be pushed into garbage collector,
 * because of dependent read reordering.
 */
static node_t *alloc_node(ptst_t *ptst)
{
    int l;
    node_t *n;
    l = get_level(ptst);
    n = (node_t*)fr_gc_alloc(ptst, gc_id[l - 1]);
    n->level = l;

    return(n);
}


/* Free a node to the garbage collector. */
static void free_node(ptst_t *ptst, node_t* n)
{
    fr_gc_free(ptst, (void *)n, gc_id[(n->level & LEVEL_MASK) - 1]);
}



/*
 * PUBLIC FUNCTIONS
 */

trans_skip *transskip_alloc(Allocator<Desc>* _descAllocator, Allocator<NodeDesc>* _nodeDescAllocator)
{
    trans_skip *l;
    node_t *n;
    int i;

    n = (node_t*)malloc(sizeof(*n) + (NUM_LEVELS-1)*sizeof(node_t *));
    memset(n, 0, sizeof(*n) + (NUM_LEVELS-1)*sizeof(node_t *));
    n->k = SENTINEL_KEYMAX;

    /*
     * Set the forward pointers of final node to other than NULL,
     * otherwise READ_FIELD() will continually execute costly barriers.
     * Note use of 0xfe -- that doesn't look like a marked value!
     */
    memset(n->next, 0xfe, NUM_LEVELS*sizeof(node_t *));

    l = (trans_skip*)malloc(sizeof(*l) + (NUM_LEVELS-1)*sizeof(node_t *));
    l->head.k = SENTINEL_KEYMIN;
    l->head.level = NUM_LEVELS;
    for ( i = 0; i < NUM_LEVELS; i++ )
    {
        l->head.next[i] = n;
    }

    l->tail = n;

    l->descAllocator = _descAllocator;
    l->nodeDescAllocator = _nodeDescAllocator;

    return(l);
}


void init_transskip_subsystem(void)
{
    int i;

    fr_init_ptst_subsystem();
    fr_init_gc_subsystem();

    for ( i = 0; i < NUM_LEVELS; i++ )
    {
        gc_id[i] = fr_gc_add_allocator(sizeof(node_t) + i*sizeof(node_t *));
    }
}

void destroy_transskip_subsystem(void)
{
    fr_destroy_gc_subsystem();
}

static inline bool help_ops(SkipList &l, Desc* desc, uint8_t opid)
{
    bool ret = true;
    // For less than 1 million nodes, it is faster not to delete nodes
    //std::vector<node_t*> deletedNodes;
    //std::vector<node_t*> insertedNodes;

    //Cyclic dependency check
    if(helpStack.Contain(desc))
    {
        if(__sync_bool_compare_and_swap(&desc->status, LIVE, ABORTED))
        {
            __sync_fetch_and_add(&g_count_abort, 1);
            __sync_fetch_and_add(&g_count_fake_abort, 1);
        }
        return false;
    }
    SkipListTransaction t;

    helpStack.Push(desc);

    l.TXBegin(t);
    while(desc->status == LIVE && ret && opid < desc->size)
    {
        const Operator& op = desc->ops[opid];

        if(op.type == O_INSERT)
        {
            node_t* n;
            ret = l.insert(op.key, t);
        }
        else if(op.type == O_DELETE)
        {
            node_t* n;
            ret = l.remove(op.key, t);
        }
        else
        {
            ret = l.contains(op.key, t);
        }

        opid++;
    }
    l.TXCommit(t);

    helpStack.Pop();

    if(ret)
    {
        if(__sync_bool_compare_and_swap(&desc->status, LIVE, COMMITTED))
        {
            __sync_fetch_and_add(&g_count_commit, 1);
        }
    }
    else
    {
        if(__sync_bool_compare_and_swap(&desc->status, LIVE, ABORTED))
        {
            __sync_fetch_and_add(&g_count_abort, 1);
        }
    }

    return ret;
}


bool execute_ops(SkipList &l, Desc* desc)
{
    helpStack.Init();

    bool ret = help_ops(l, desc, 0);

    return ret;
}

void transskip_print(trans_skip* l)
{
    node_t* curr = l->head.next[0];

    while(curr != l->tail)
    {
        printf("Node [%p] Key [%u] Status [%s]\n", curr, INTERNAL_TO_CALLER_KEY(curr->k), IsKeyExist(CLR_MARKD(curr->nodeDesc))? "Exist":"Inexist");
        curr = (node_t*)get_unmarked_ref(curr->next[0]); 
    }
}

void transskip_free(trans_skip* l)
{
    printf("Total commit %u, abort (total/fake) %u/%u\n", g_count_commit, g_count_abort, g_count_fake_abort);

    //transskip_print(l);
}

void ResetMetrics(trans_skip* l)
{
    g_count_commit = 0;
    g_count_abort = 0;
    g_count_fake_abort = 0;
}
