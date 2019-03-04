#include <iostream>
#include <time.h>
#include <random>
#include <thread>
#include <atomic>

#include "tskiplist/Utils.h"
#include "tskiplist/TSkipList.h"

using namespace std;

enum WorkloadType
{
    READ_ONLY = 0,
    MIXED = 1,
    UPDATE_ONLY = 2
};

unsigned int constexpr WARM_UP_NUM_KEYS = 100000;
unsigned int constexpr MIN_KEY_VAL = 1;
unsigned int constexpr MAX_KEY_VAL = 1000000;
unsigned int constexpr TIMEOUT = 10;


void warmUp(SkipList & sl, default_random_engine & generator)
{
    uniform_int_distribution<int> distribution(MIN_KEY_VAL, MAX_KEY_VAL);

    for (auto i = 0; i < WARM_UP_NUM_KEYS; i++) {
        int percent = (int)((i / float(WARM_UP_NUM_KEYS)) * 100);
        printf("\rWarming up... %d%%", percent);

        SkipListTransaction trans;
        sl.TXBegin(trans);
        sl.insert(distribution(generator), trans);
        sl.TXCommit(trans);
    }

    printf("\nList sum: %d\n", sl.index.sum());
}

void chooseOps(WorkloadType wtype, uint32_t numOps,
               vector<OperationType> & outOps)
{
    switch (wtype) {
    case READ_ONLY:
        for (uint32_t i = 0; i < numOps; i++) {
            outOps.push_back(OperationType::CONTAINS);
        }
        break;
    case UPDATE_ONLY: {
        const uint32_t halfPoint = (uint32_t)(numOps / 2.0);
        for (uint32_t i = 0; i < halfPoint; i++) {
            outOps.push_back(OperationType::INSERT);
        }
        for (uint32_t i = halfPoint; i < numOps; i++) {
            outOps.push_back(OperationType::REMOVE);
        }
        break;
    }
    case MIXED: {
        const uint32_t halfPoint = (uint32_t)(numOps / 2.0);
        const uint32_t threeQuarters = (uint32_t)(numOps * 3.0 / 4.0);
        for (uint32_t i = 0; i < halfPoint; i++) {
            outOps.push_back(OperationType::CONTAINS);
        }
        for (uint32_t i = halfPoint; i < threeQuarters; i++) {
            outOps.push_back(OperationType::REMOVE);
        }
        for (uint32_t i = threeQuarters; i < numOps; i++) {
            outOps.push_back(OperationType::INSERT);
        }
        break;
    }
    }
}

void worker(SkipList * sl, default_random_engine * generator,
            atomic<uint32_t> * opsCounter, atomic<uint32_t> * abortCounter,
            WorkloadType wtype, time_t end)
{
    uniform_int_distribution<int> key_distribution(MIN_KEY_VAL, MAX_KEY_VAL);
    uniform_int_distribution<uint32_t> transaction_distribution(1, 7);

    while (time(NULL) < end) {
        int numOps = transaction_distribution(*generator);

        vector<OperationType> ops;
        chooseOps(wtype, numOps, ops);

        SkipListTransaction trans;
        sl->TXBegin(trans);
        try {
            for (uint32_t i = 0; i < numOps; i++) {
                sl->contains(key_distribution(*generator), trans);
            }
            sl->TXCommit(trans);
            atomic_fetch_add<uint32_t>(opsCounter, numOps);
        } catch (AbortTransactionException &) {
            atomic_fetch_add<uint32_t>(abortCounter, 1);
        }
    }
}

int main(int argc, char * argv[])
{
    if (argc != 3) {
        cout << "Invalid number of parameters" << endl;
        cout << "Usage: " << argv[0] << " <WORKLOAD_TYPE> <NUM_THREADS>" << endl;
        cout << "Workload types: 0 = READ_ONLY, 1 = MIXED, 2 = UPDATE_ONLY" << endl;
        return 1;
    }

    srand(time(NULL));

    WorkloadType wtype = (WorkloadType)(atoi(argv[1]));

    SkipList sl;
    default_random_engine generator;
    warmUp(sl, generator);

    uint32_t numThreads = atoi(argv[2]);
    vector<thread> threads;

    time_t end = time(NULL) + TIMEOUT;

    atomic<uint32_t> opsCounter(0);
    atomic<uint32_t> abortCounter(0);

    for (uint32_t i = 0; i < numThreads; i++) {
        threads.push_back(thread(worker, &sl, &generator, &opsCounter, &abortCounter,
                                 wtype, end));
    }

    for (auto & t : threads) {
        t.join();
    }

    cout << "Num ops: " << opsCounter << endl;
    cout << "Num aborts: " << abortCounter << endl;
    return 0;
}