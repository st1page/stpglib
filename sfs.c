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
    for(int i=0; i < recordMeta->len; i++){
        uint8_t x = (uint8_t)recordMeta->buf[i];
        recordSize += x ? x: 0;        
    }
    return recordSize;
}

inline int calcTableSize(uint32_t storeSize, const SFSVarchar *recordMeta){
    return sizeof(SFSTableHdr) + storeSize + sizeof(SFSVarchar) + recordMeta->len;
}

inline const SFSVarchar* getRecordMeta(SFSTableHdr *table){
    return (SFSVarchar*)((char*)table + table->recordMetaOffset);
}

inline const uint32_t getStorSize(SFSTableHdr *table){
    return table->size - sizeof(SFSTableHdr) - sizeof(SFSVarchar) - getRecordMeta(table)->len;
}
int sfsTableCons(SFSTableHdr *table, 
                uint32_t initStorSize, 
                const SFSVarchar *recordMeta,
                SFSDatabase *db){
    if(initStorSize == 0) initStorSize = 16 * calcRecordSize(recordMeta);
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
    if(initStorSize == 0) initStorSize = 16 * calcRecordSize(recordMeta);
    uint32_t tableSize = calcTableSize(initStorSize, recordMeta); 
    SFSTableHdr *table = malloc(tableSize);
    sfsTableCons(table, initStorSize, recordMeta, db);
    return table;
}
int sfsTableRelease(SFSTableHdr *table){
    free(table);
    return 1;
}
SFSTableHdr* sfsTableReserve(SFSTableHdr *table, uint32_t storSize){
    uint32_t newTableSize = calcTableSize(storSize, getRecordMeta(table));
    uint32_t oldTableSize = table->size;
    if(newTableSize <= oldTableSize) return table;
    uint32_t deltaOffset = newTableSize - oldTableSize;
    SFSTableHdr *newTable = sfsTableCreate(storSize, getRecordMeta(table), table->database.ptr);
    
    memcpy(newTable, table, sizeof(SFSTableHdr) + table->recordNum * table->recordSize);
    uint32_t tailLen = table->size - table->lastVarcharOffset;
    memcpy(offsetPtr(newTable, newTableSize - tailLen),
            offsetPtr(table, table->lastVarcharOffset),
            tailLen);
    
    newTable->freeSpace += deltaOffset;
    newTable->lastVarcharOffset += deltaOffset;
    newTable->recordMetaOffset += deltaOffset;
    
    const SFSVarchar *recordMeta = getRecordMeta(newTable);
    char* st = newTable->buf;
    for(int i=0; i < recordMeta->len; i++){
        uint8_t type = (uint8_t)recordMeta->buf[i];
        if(type){
            st = offsetPtr(st, type);
        } else {
            uint32_t *cur = (uint32_t*)st;
            for(uint32_t j=0; j < newTable->recordNum; j++){
                *cur = offsetPtr(*cur, deltaOffset);
                cur = offsetPtr(cur, newTable->recordSize);
            }
            st = offsetPtr(st, 4);
        }
    }    
    
    free(table);
    (newTable->database.ptr)->size += deltaOffset;
    return newTable;
}
inline void recordVarcharToOffset(SFSTableHdr *table, void *record){
    const SFSVarchar *recordMeta = getRecordMeta(table);
    uint32_t *cur = (uint32_t*)record;
    for(int i=0; i < recordMeta->len; i++){
        uint8_t type = (uint8_t)recordMeta->buf[i];
        if(type){
            cur = offsetPtr(cur, type);
        } else {
            *cur = ptrOffset(table, (void *)(*cur) );
            cur = offsetPtr(cur, 4);
        }
    }    
}
inline void recordVarcharToPtr(SFSTableHdr *table, void *record){
    const SFSVarchar *recordMeta = getRecordMeta(table);
    uint32_t *cur = (uint32_t*)record;
    for(int i=0; i < recordMeta->len; i++){
        uint8_t type = (uint8_t)recordMeta->buf[i];
        if(type){
            cur = offsetPtr(cur, type);
        } else {
            *cur = offsetPtr(table, *cur);
            cur = offsetPtr(cur, 4);
        }
    }    
}
void sfsTableForeach(SFSTableHdr *table, void (*fun)(void*)){
    void* record = table->buf;
    for(int i = 0; i < table->recordNum; i++){
        fun(record);
        record = offsetPtr(record, table->recordSize);
    }
}
inline SFSTableHdr* tableExpand(SFSTableHdr *table, uint32_t addSize){
    uint32_t oldSize = getStorSize(table);
    uint32_t newSize;
    if(oldSize + table->freeSpace < addSize) newSize = oldSize *2;
    else newSize = oldSize + addSize;
    table = sfsTableReserve(table, newSize);
    return table;
}
void* sfsTableAddRecord(SFSTableHdr *table){
    if(table->freeSpace < table->recordSize) tableExpand(table, table->recordSize);
    void* retPtr = offsetPtr(table->buf, table->recordNum * table->recordSize);
    table->recordNum++;
    table->freeSpace -= table->recordSize;
    return retPtr;
}
SFSVarchar* sfsTableAddVarchar(SFSTableHdr *table, uint32_t varcharLen, const char* src){
    if(table->freeSpace < table->recordSize) tableExpand(table, varcharLen + sizeof(SFSVarchar));
    SFSVarchar *retPtr = offsetPtr(table->buf, table->lastVarcharOffset);
    retPtr = negOffsetPtr(retPtr, varcharLen+sizeof(SFSVarchar));
    table->lastVarcharOffset = ptrOffset(retPtr, table);
    table->varcharNum++;
    return retPtr;
}

SFSDatabase* sfsDatabaseCreate(uint32_t storSize){
    SFSDatabase *db = malloc(sizeof(SFSDatabase)+storSize);
    db->magic = MAGIC;
    db->version = VERSION;
    db->size = sizeof(SFSDatabase)+storSize;
    db->tableNum = 0;
    return db;
}
void sfsDatabaseRelease(SFSDatabase* db){
    for(int i=0; i<db->tableNum; i++){
        sfsTableRelease(db->table[i].ptr);
    }
    free(db);
}
void sfsDatabaseSave(char *fileName, SFSDatabase* db);
SFSDatabase* sfsDatabaseCreateLoad(char *fileName);

SFSTableHdr* sfsDatabaseAddTable(SFSDatabase *db, uint32_t initStorSize, const SFSVarchar *recordMeta){
    if(initStorSize == 0) initStorSize = 16 * calcRecordSize(recordMeta);
    uint32_t tableSize = calcTableSize(initStorSize, recordMeta);
    SFSTableHdr *table = sfsTableCreate(initStorSize, recordMeta, db);
    db->table[db->tableNum].ptr = table;
    db->tableNum++;
    return table;
}

// return the lastest err
char *sfsErrMsg(){
    return errmsg;  
}



