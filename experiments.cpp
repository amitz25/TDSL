#include <iostream>
#include <time.h>
#include <random>

#include "Utils.h"
#include "TSkipList.h"

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


void warmUp(SkipList & sl)
{


    default_random_engine generator;
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

int main(int argc, char * argv[])
{
    if (argc != 2) {
        cout << "Invalid number of parameters" << endl;
        return 1;
    }

    srand(time(NULL));

    WorkloadType wtype = (WorkloadType)(atoi(argv[1]));

    SkipList sl;
    warmUp(sl);
    return 0;
}