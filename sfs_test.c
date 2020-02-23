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

TEST tableRAII(void) {
    CHECK_CALL(talbeAinit()); /* <3 */
    CHECK_CALL(tableDestory()); /* </3 */
    PASS();
}

SUITE(table_suite) {
    initStorSize = ArecordSize * 5;
    RUN_TEST(tableRAII);


}
int main(int argc, char **argv){
    //    for(int i=0;i<A_meta->len;i++) printf("%d\n", A_meta->buf[i]); 
    GREATEST_MAIN_BEGIN();     /* command-line arguments, initialization. */

    RUN_SUITE(table_suite);
//    RUN_SUITE(file_suite);
    
    GREATEST_MAIN_END();
    return 0;
}