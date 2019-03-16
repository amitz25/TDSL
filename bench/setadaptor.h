#ifndef SETADAPTOR_H
#define SETADAPTOR_H

#include "transskip.h"
#include "common/allocator.h"

enum SetOpType
{
    S_FIND = 0,
    S_INSERT,
    S_DELETE
};

struct SetOperator
{
    uint8_t type;
    uint32_t key;
};

enum SetOpStatus
{
    LIVE = 0,
    COMMITTED,
    ABORTED
};

typedef std::vector<SetOperator> SetOpArray;

template<typename T>
class SetAdaptor
{
};

template<>
class SetAdaptor<trans_skip>
{
public:
    SetAdaptor(uint64_t cap, uint64_t threadCount, uint32_t transSize)
        : m_descAllocator(cap * threadCount * Desc::SizeOf(transSize), threadCount, Desc::SizeOf(transSize))
        , m_nodeDescAllocator(cap * threadCount *  sizeof(NodeDesc) * transSize, threadCount, sizeof(NodeDesc))
    { 
        m_skiplist = transskip_alloc(&m_descAllocator, &m_nodeDescAllocator);
        init_transskip_subsystem(); 
    }

    ~SetAdaptor()
    {
        transskip_free(m_skiplist);
    }

    void Init()
    {
        m_descAllocator.Init();
        m_nodeDescAllocator.Init();
        ResetMetrics(m_skiplist);
    }

    void Uninit()
    {
        destroy_transskip_subsystem(); 
    }

    bool ExecuteOps(const SetOpArray& ops)
    {
        //TransList::Desc* desc = m_list.AllocateDesc(ops.size());
        Desc* desc = m_descAllocator.Alloc();
        desc->size = ops.size();
        desc->status = LIVE;

        for(uint32_t i = 0; i < ops.size(); ++i)
        {
            desc->ops[i].type = ops[i].type; 
            desc->ops[i].key = ops[i].key; 
        }

        return execute_ops(m_skiplist, desc);
    }

private:
    Allocator<Desc> m_descAllocator;
    Allocator<NodeDesc> m_nodeDescAllocator;
    trans_skip* m_skiplist;
};

#endif /* end of include guard: SETADAPTOR_H */
