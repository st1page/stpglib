#include <stdio.h>

#include "sfs.h"
#include "./greatest/greatest.h"

#pragma pack(1)
struct A{
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
};
char A_meta_c[] = {5, 0, 0, 0, 1, 4, 0, 10, 0};
SFSVarchar *A_meta = (SFSVarchar *)A_meta_c;
/*    .len = 5,
    .buf = {1,4,0,10,0} */


int main(){
//    for(int i=0;i<A_meta->len;i++) printf("%d\n", A_meta->buf[i]); 

}