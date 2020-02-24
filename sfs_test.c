#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "./sfs.h"
#include "./greatest/greatest.h"

GREATEST_MAIN_DEFS();

#pragma pack(1)
typedef struct A{
    int8_t x0_1;
    uint32_t x1_4;
    union{
        SFSVarchar *ptr;
        uint32_t offset;
    }x2_v;
    char x3_10[10];
    union{
        SFSVarchar *ptr;
        uint32_t offset;
    }x4_v;
}A;
char AMeta_c[] = {5, 0, 0, 0, 1, 4, 0, 10, 0};
SFSVarchar *AMeta = (SFSVarchar *)AMeta_c;
/*    .len = 5,
    .buf = {1,4,0,10,0} */
const uint32_t ArecordSize = sizeof(A);
SFSDatabase *db = NULL;
SFSTableHdr *table = NULL;
uint32_t initStorSize = 0;

static enum greatest_test_res talbeAinit(){
    table = sfsTableCreate(initStorSize, AMeta, db);
    if(table==NULL) FAILm(sfsErrMsg());
    PASS();
}

static enum greatest_test_res tableDestory(){
    if(!sfsTableRelease(table)) FAILm(sfsErrMsg());
    table = NULL;
    PASS();
}

TEST tableInitTest(void) {
    initStorSize = ArecordSize * 5;
    CHECK_CALL(talbeAinit()); 
    ASSERT_EQ_FMT(ArecordSize, table->recordSize, "%d");
    ASSERT_EQ_FMT(initStorSize, table->freeSpace, "%d");
    ASSERT_EQ_FMT(0, table->recordNum, "%d");
    ASSERT_EQ_FMT(0, table->varcharNum, "%d");
    ASSERT_EQ_FMT(table->lastVarcharOffset, table->recordMetaOffset, "%d");
    ASSERT_EQ_FMT(sizeof(AMeta_c), table->size - table->recordMetaOffset, "%d");
    ASSERT_MEM_EQ(AMeta_c, (char*)table + table->recordMetaOffset , sizeof(AMeta_c));
    CHECK_CALL(tableDestory()); 
    PASS();
}
TEST tableStaticMem(void) {
    initStorSize = ArecordSize * 5;
    uint32_t storSize = initStorSize;
    CHECK_CALL(talbeAinit()); 

    A* addrs[5];
    for(int i=0;i<5;i++){
        ASSERT_EQ_FMT(storSize, table->freeSpace, "%d");
        addrs[i] = sfsTableAddRecord(table);
        storSize -= ArecordSize;
        ASSERT_EQ_FMT(i+1, table->recordNum, "%d");
    }
    ASSERT_EQ_FMT(0, table->freeSpace, "%d");
    ASSERT_EQ_FMT(0, (char*)addrs[0] - (char*)table->buf, "%d");
    for(int i=1;i<5;i++){
        ASSERT_EQ_FMT(ArecordSize, (char*)addrs[i] - (char*)addrs[i-1], "%d");
    }
    ASSERT_EQ_FMT(ArecordSize, table->lastVarcharOffset - ((char*)addrs[4] - (char*)table) , "%d");

    CHECK_CALL(tableDestory()); 
    PASS();
}

SUITE(table_suite) {
    RUN_TEST(tableInitTest);
    RUN_TEST(tableStaticMem);


}
int main(int argc, char **argv){
    //    for(int i=0;i<A_meta->len;i++) printf("%d\n", A_meta->buf[i]); 
    GREATEST_MAIN_BEGIN();     /* command-line arguments, initialization. */

    RUN_SUITE(table_suite);
//    RUN_SUITE(file_suite);
    
    GREATEST_MAIN_END();
    return 0;
}