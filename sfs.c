#include "sfs.h"

#include <stdlib.h>
#include <string.h>

#define MAGIC   (0x534653aa)
#define VERSION (0x01000000)

char *errmsg = "";


inline uint32_t ptrOffset(void *x, void *y){
    return (char*)y > (char*)x ?
            (char*)y - (char*)x:
            (char*)x - (char*)y;
}

inline void* offsetPtr(void* x, uint32_t y){
    return (char*)x + y;
}
inline void* negOffsetPtr(void* x, uint32_t y){
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

inline int calcRecordSize(const SFSVarchar *recordMeta){
    uint32_t recordSize = 0;
    for(int i=0; i<recordMeta->len; i++){
        uint8_t x = (uint8_t)recordMeta->buf[i];
        recordSize += x ? x: 0;        
    }
    return recordSize;
}

inline int calcTableSize(uint32_t storeSize, const SFSVarchar *recordMeta){
    return sizeof(SFSTableHdr) + storeSize + sizeof(SFSVarchar) + recordMeta->len;
}

inline SFSVarchar* getRecordMeta(SFSTableHdr *table){
    return (SFSVarchar*)((char*)table + table->recordMetaOffset);
}
int sfsTableCons(SFSTableHdr *table, 
                uint32_t initStorSize, 
                const SFSVarchar *recordMeta,
                SFSDatabase *db){
    table->size = calcTableSize(initStorSize, recordMeta);
    table->freeSpace = initStorSize;
    table->varcharNum = -1; // to insert recordMeta
    table->recordNum = 0;
    table->recordSize = calcRecordSize(recordMeta);
    table->lastVarcharOffset = table->size;
    sfsTableAddVarchar(table, recordMeta->len, recordMeta->buf);
    table->recordMetaOffset = table->lastVarcharOffset;
    table->database.ptr = db;
    return 1;
}

SFSTableHdr* sfsTableCreate(uint32_t initStorSize, 
                const SFSVarchar *recordMeta,
                SFSDatabase *db){
    uint32_t tableSize = calcTableSize(initStorSize, recordMeta); 
    SFSTableHdr *table = malloc(tableSize);
    sfsTableCons(table, initStorSize, recordMeta, db);
    return table;
}
int sfsTableRelease(SFSTableHdr *table){
    free(table);
    return 1;
}
//TODO
void sfsTableForeach(SFSTableHdr *table, void (*fun)(void*)){

}
SFSTableHdr* sfsTableReserve(SFSTableHdr *table, uint32_t storSize){
    uint32_t newTableSize = calcTableSize(storSize, getRecordMeta(table));
    uint32_t oldTableSize = table->size;
    if(newTableSize <= oldTableSize) return table;
    // TODO
}

void* sfsTableAddRecord(SFSTableHdr *table){
    void* retPtr = offsetPtr(table->buf, table->recordNum * table->recordSize);
    table->recordNum++;
    return retPtr;
}
SFSVarchar* sfsTableAddVarchar(SFSTableHdr *table, uint32_t varcharLen, const char* src){
    SFSVarchar *retPtr = offsetPtr(table->buf, table->lastVarcharOffset);
    retPtr = negOffsetPtr(retPtr, varcharLen+sizeof(SFSVarchar));
    table->lastVarcharOffset = ptrOffset(retPtr, table);
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



