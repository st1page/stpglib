#include "sfs.h"

#include <stdlib.h>
#include <string.h>

#define MAGIC (0x534653aa)
#define VERSION (1)

char *errmsg = "";


inline uint32_t _ptrOffset(void *x, void *y){
    return (char*)y > (char*)x ?
            (char*)y - (char*)x:
            (char*)x - (char*)y;
}

inline void* _offsetPtr(void* x, uint32_t y){
    return (char*)x + y;
}
inline void* _negOffsetPtr(void* x, uint32_t y){
    return (char*)x - y;
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
int sfsTablewCons(SFSTableHdr *table, uint32_t storSize, SFSVarchar *recordMeta){
    table->size = _calcTableSize(storSize, recordMeta);
    table->freeSpace = storSize;
    table->varcharNum = -1; // to insert recordMeta
    table->recordNum = 0;
    table->size = _calcRecordSize(recordMeta);
    table->lastVarcharOffset = table->size;
    sfsTableAddVarchar(table, recordMeta->len, recordMeta->buf);
    table->recordMetaOffset = table->lastVarcharOffset;
    return 1;
}

void* sfsTableAddRecord(SFSTableHdr *table){
    void* retPtr = _offsetPtr(table->buf, table->recordNum * table->recordSize);
    table->recordNum++;
    return retPtr;
}
SFSVarchar* sfsTableAddVarchar(SFSTableHdr *table, uint32_t varcharLen, const char* src){
    SFSVarchar *retPtr = _offsetPtr(table->buf, table->lastVarcharOffset);
    retPtr = _negOffsetPtr(retPtr, varcharLen+sizeof(SFSVarchar));
    table->lastVarcharOffset = _ptrOffset(retPtr, table);
    table->varcharNum++;
    return retPtr;
}

SFSTableHdr* sfsFileAddTable(SFSFileHdr *file, uint32_t storSize, SFSVarchar *recordMeta){
    uint32_t tableSize = _calcTableSize(storSize, recordMeta);
    uint32_t tableOffset;
    if(file->tableNum) {
        tableOffset = file->tableOffset[file->tableNum - 1];
        SFSTableHdr *lastTable = _offsetPtr(file, tableOffset);
        tableOffset += lastTable->size;
    } else tableOffset = sizeof(SFSFileHdr);
    SFSTableHdr *table = _offsetPtr(file, tableOffset);
    sfsTablewCons(table, storSize, recordMeta);

    file->tableNum++;
    file->tableOffset[file->tableNum - 1] = tableOffset;
    return table;
}
SFSFileHdr* sfsFileCreate(uint32_t storSize){
    SFSFileHdr *file = (SFSFileHdr*)malloc(sizeof(SFSFileHdr)+storSize);
    file->magic = MAGIC;
    file->version = VERSION;
    file->size = sizeof(SFSFileHdr)+storSize;
    file->tableNum = 0;
    return file;
}
void sfsFileRelease(SFSFileHdr* file){
    free(file);
}
void sfsFileSave(char *fileName, SFSFileHdr* sfsFile){
    
}
SFSFileHdr* sfsFileLoad(char *fileName);


// return the lastest err
char *sfsErrMsg(){
    return errmsg;  
}



