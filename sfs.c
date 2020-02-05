#include "sfs.h"

#include <stdlib.h>
#include <string.h>
char *errmsg;

inline uint32_t _ptrOffset(void *x, void *y){
    return (char*)y > (char*)x ?
            (char*)y - (char*)x:
            (char*)x - (char*)y;
}

inline void* _offsetPtr(void* x, uint32_t y){
    return (char*)x + y;
}

int sfsVarcharCons(SFSVarchar *varchar, const char* src){
    memcpy(varchar->buf, src, varchar->len);
    return 1;
}

SFSVarchar* sfsVarcharCreate(uint32_t varcharLen, const char* src){
    SFSVarchar *varchar = (SFSVarchar*)malloc(sizeof(SFSVarchar)+varcharLen);
    varchar->len = varcharLen; 
    if(src) memcpy(varchar->buf, src, varcharLen);
    return varchar;
}

int sfsVarcharRelease(SFSVarchar *varchar){
    free(varchar);
    return 1;
}

inline int _calcRecordSize(SFSVarchar *recordMeta){
    uint32_t recordSize = 0;
    for(int i=0; i<recordMeta->len; i++){
        uint8_t x = (uint8_t)recordMeta->buf[i];
        recordSize += x ? x: 0;        
    }
    return recordSize;
}

inline int _calcTableSize(uint32_t storeSize, SFSVarchar *recordMeta){
    return sizeof(SFSTableHdr) + storeSize + sizeof(SFSVarchar) + recordMeta->len;
}

inline SFSVarchar* _getrecordMeta(SFSTableHdr *table){
    return (SFSVarchar*)((char*)table + table->recordMetaOffset);
}
//TODO
int sfsTablewCons(SFSTableHdr *table, uint32_t storSize, SFSVarchar *recordMeta){

    return 1;
}

void* sfsTableAddRecord(SFSTableHdr *table);
SFSVarchar* sfsTablAddVarchar(SFSTableHdr *table, uint32_t varcharSize, const char* src);

SFSTableHdr* sfsFileAddTable(SFSFileHdr *file, uint32_t storSize, SFSVarchar *recordMeta);
SFSFileHdr* sfsFileCreate();
SFSFileHdr* sfsFileLoad(char *fileName);
void sfsFileRelease(SFSFileHdr* sfsFile);
void sfsFileSave(char *fileName, SFSFileHdr* sfsFile);

// return the lastest err
char *sfsErrMsg(){
    return errmsg;  
}



