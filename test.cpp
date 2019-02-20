#include "gtest/gtest.h"

#include "Index.h"
#include "SkipList.h"

class TDSLTest : public ::testing::Test
{
public:
    TDSLTest()
    {

    }
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(TDSLTest, IndexBasic)
{
    Index index(0);
    ASSERT_TRUE(index.insert(5));
    ASSERT_TRUE(index.insert(10));
    ASSERT_TRUE(index.insert(7));

    ASSERT_TRUE(index.contains(5));
    ASSERT_TRUE(index.contains(7));
    ASSERT_TRUE(index.contains(10));

    ASSERT_EQ(index.sum(), 22);

    ASSERT_FALSE(index.remove(3));
    ASSERT_FALSE(index.insert(5));

    // Regular removal
    ASSERT_TRUE(index.remove(7));
    ASSERT_EQ(index.sum(), 15);
    ASSERT_FALSE(index.contains(7));

    // Head removal
    ASSERT_TRUE(index.remove(5));
    ASSERT_EQ(index.sum(), 10);
    ASSERT_FALSE(index.contains(5));

    // Head insertion
    ASSERT_TRUE(index.insert(3));
    ASSERT_TRUE(index.contains(3));
    ASSERT_EQ(index.sum(), 13);
}

void initSkipList(SkipList & sl)
{
    SkipListTransaction trans;
    sl.TXBegin(trans);
    ASSERT_TRUE(sl.insert(0, trans));
    ASSERT_TRUE(sl.insert(2, trans));
    ASSERT_TRUE(sl.insert(4, trans));
    ASSERT_TRUE(sl.insert(10, trans));
    ASSERT_TRUE(sl.insert(15, trans));
    ASSERT_TRUE(sl.insert(20, trans));
    ASSERT_NO_THROW(sl.TXCommit(trans));
}

TEST_F(TDSLTest, SkipListCorrectness)
{
    SkipList sl;
    initSkipList(sl);
    ASSERT_EQ(sl.index.sum(), 51);

    {
        SkipListTransaction trans;
        sl.TXBegin(trans);
        ASSERT_TRUE(sl.remove(2, trans));
        ASSERT_NO_THROW(sl.TXCommit(trans));
        ASSERT_EQ(sl.index.sum(), 49);
    }

    {
        SkipListTransaction trans;
        sl.TXBegin(trans);
        ASSERT_TRUE(sl.remove(4, trans));
        ASSERT_TRUE(sl.remove(10, trans));
        ASSERT_TRUE(sl.remove(0, trans));
        ASSERT_NO_THROW(sl.TXCommit(trans));
        ASSERT_EQ(sl.index.sum(), 35);
    }

    {
        SkipListTransaction trans;
        sl.TXBegin(trans);
        ASSERT_TRUE(sl.insert(17, trans));
        ASSERT_TRUE(sl.insert(18, trans));
        ASSERT_TRUE(sl.remove(17, trans));
        ASSERT_NO_THROW(sl.TXCommit(trans));
        ASSERT_EQ(sl.index.sum(), 53);
    }

    {
        SkipListTransaction trans;
        sl.TXBegin(trans);
        ASSERT_FALSE(sl.remove(17, trans));
        ASSERT_FALSE(sl.insert(15, trans));
        ASSERT_NO_THROW(sl.TXCommit(trans));
        ASSERT_EQ(sl.index.sum(), 53);
    }
}

TEST_F(TDSLTest, SkipListConcurrentInsert)
{
    SkipList sl;
    initSkipList(sl);

    SkipListTransaction trans1;
    SkipListTransaction trans2;
    sl.TXBegin(trans1);
    sl.TXBegin(trans2);
    ASSERT_NO_THROW(sl.insert(1, trans1));
    ASSERT_NO_THROW(sl.insert(3, trans2));
    ASSERT_NO_THROW(sl.TXCommit(trans1));
    ASSERT_NO_THROW(sl.TXCommit(trans2));
}

TEST_F(TDSLTest, SkipListConflictingInsert)
{
    SkipList sl;
    initSkipList(sl);

    {
        // Conflict during commit
        SkipListTransaction trans1;
        SkipListTransaction trans2;
        sl.TXBegin(trans1);
        sl.TXBegin(trans2);
        ASSERT_TRUE(sl.insert(11, trans1));
        ASSERT_TRUE(sl.insert(12, trans2));
        ASSERT_NO_THROW(sl.TXCommit(trans1));
        ASSERT_EQ(sl.index.sum(), 62);
        ASSERT_THROW(sl.TXCommit(trans2), AbortTransactionException);
        ASSERT_EQ(sl.index.sum(), 62);
    }

    {
        // Conflict during insert
        SkipListTransaction trans1;
        SkipListTransaction trans2;
        sl.TXBegin(trans1);
        sl.TXBegin(trans2);
        ASSERT_TRUE(sl.insert(12, trans1));
        ASSERT_NO_THROW(sl.TXCommit(trans1));
        ASSERT_EQ(sl.index.sum(), 74);
        ASSERT_THROW(sl.insert(13, trans2), AbortTransactionException);
        ASSERT_EQ(sl.index.sum(), 74);
    }

    // Ensure insert still works
    SkipListTransaction trans;
    sl.TXBegin(trans);
    ASSERT_TRUE(sl.insert(13, trans));
    ASSERT_NO_THROW(sl.TXCommit(trans));
    ASSERT_EQ(sl.index.sum(), 87);
}

TEST_F(TDSLTest, SkipListConcurrentRemove)
{
    SkipList sl;
    ASSERT_NO_THROW(initSkipList(sl));

    SkipListTransaction trans1;
    SkipListTransaction trans2;
    sl.TXBegin(trans1);
    sl.TXBegin(trans2);
    ASSERT_NO_THROW(sl.remove(10, trans1));
    ASSERT_NO_THROW(sl.remove(2, trans2));
    ASSERT_NO_THROW(sl.TXCommit(trans1));
    ASSERT_NO_THROW(sl.TXCommit(trans2));
}

TEST_F(TDSLTest, SkipListConflictingRemove)
{
    SkipList sl;
    ASSERT_NO_THROW(initSkipList(sl));

    {
        // Conflict during commit
        SkipListTransaction trans1;
        SkipListTransaction trans2;
        sl.TXBegin(trans1);
        sl.TXBegin(trans2);
        ASSERT_TRUE(sl.remove(10, trans1));
        ASSERT_TRUE(sl.remove(4, trans2));
        ASSERT_NO_THROW(sl.TXCommit(trans1));
        ASSERT_EQ(sl.index.sum(), 41);
        ASSERT_THROW(sl.TXCommit(trans2), AbortTransactionException);
        ASSERT_EQ(sl.index.sum(), 41);
    }


    {
        // Conflict during remove
        SkipListTransaction trans1;
        SkipListTransaction trans2;
        sl.TXBegin(trans1);
        sl.TXBegin(trans2);
        ASSERT_TRUE(sl.remove(4, trans1));
        ASSERT_NO_THROW(sl.TXCommit(trans1));
        ASSERT_EQ(sl.index.sum(), 37);
        ASSERT_THROW(sl.remove(2, trans2), AbortTransactionException);
        ASSERT_EQ(sl.index.sum(), 37);
    }

    // Ensure removal still works
    SkipListTransaction trans;
    sl.TXBegin(trans);
    ASSERT_TRUE(sl.remove(2, trans));
    ASSERT_NO_THROW(sl.TXCommit(trans));
    ASSERT_EQ(sl.index.sum(), 35);
}


int main(int argc, char ** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
