#include "sfs.h"

#include "crc32.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAGIC   (0x534653aa)
#define VERSION (0x01000000)

char errmsg[8192];

uint32_t ptrOffset(void *x, void *y){
    return (char*)y > (char*)x ?
            (char*)y - (char*)x:
            (char*)x - (char*)y;
}

void* offsetPtr(void* x, uint32_t y){
    return (char*)x + y;
}
void* negOffsetPtr(void* x, uint32_t y){
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

uint32_t calcRecordSize(const SFSVarchar *recordMeta){
    uint32_t recordSize = 0;
    for(int i=0; i < recordMeta->len; i++){
        uint8_t x = (uint8_t)recordMeta->buf[i];
        recordSize += x ? x: 4;        
    }
    return recordSize;
}

int calcTableSize(uint32_t storeSize, const SFSVarchar *recordMeta){
    return sizeof(SFSTable) + storeSize + sizeof(SFSVarchar) + recordMeta->len;
}
int sfsTableCons(SFSTable *table, 
                uint32_t initStorSize, 
                const SFSVarchar *recordMeta,
                SFSDatabase *db){
    if(initStorSize == 0) initStorSize = 16 * calcRecordSize(recordMeta);
    table->size = calcTableSize(initStorSize, recordMeta);
    table->storSize = initStorSize;
    table->freeSpace = initStorSize;
    table->varcharNum = -1; // to insert recordMeta
    table->recordNum = 0;
    table->recordSize = calcRecordSize(recordMeta);
    table->lastVarchar= offsetPtr(table, table->size);
    sfsTableAddVarchar(&table, recordMeta->len, recordMeta->buf);
    table->recordMeta = table->lastVarchar;
    table->database = db;
    return 1;
}

SFSTable* sfsTableCreate(uint32_t initStorSize, 
                const SFSVarchar *recordMeta,
                SFSDatabase *db){
    printf("create %d\n",initStorSize);
    if(initStorSize == 0) initStorSize = 16 * calcRecordSize(recordMeta);
    uint32_t tableSize = calcTableSize(initStorSize, recordMeta); 
    SFSTable *table = malloc(tableSize);
    sfsTableCons(table, initStorSize, recordMeta, db);
    return table;
}
int sfsTableRelease(SFSTable *table){
    free(table);
    return 1;
}
int sfsTableReserve(SFSTable **ptable, uint32_t storSize){
    SFSTable *table = *ptable;
    uint32_t newTableSize = calcTableSize(storSize, table->recordMeta);
    uint32_t oldTableSize = table->size;
    if(newTableSize <= oldTableSize) return 1;
    uint32_t delta = newTableSize - oldTableSize;

    const SFSVarchar *recordMeta = table->recordMeta;
    SFSTable *newTable = sfsTableCreate(storSize, recordMeta, table->database);
    
    memcpy(newTable, table, sizeof(SFSTable) + table->recordNum * table->recordSize);
    uint32_t tailLen = table->size - ptrOffset(table, table->lastVarchar);
    memcpy(offsetPtr(newTable, newTableSize - tailLen),
            table->lastVarchar,
            tailLen);
    
    newTable->size = newTableSize;
    newTable->storSize = storSize;
    newTable->freeSpace += delta;
    newTable->lastVarchar = offsetPtr(newTable, newTableSize - tailLen);
    newTable->recordMeta = 
        offsetPtr(newTable, newTableSize - sizeof(SFSVarchar) - recordMeta->len);

    char* st = newTable->buf;
    for(uint32_t i=0; i < recordMeta->len; i++){
        uint8_t type = (uint8_t)recordMeta->buf[i];
        if(type){
            st = offsetPtr(st, type);
        } else {
            void* *cur = (void **)st;
            for(uint32_t j=0; j < newTable->recordNum; j++){
                uint32_t offset = ptrOffset(table, (*cur) ) + delta;
                *cur = offsetPtr(newTable, offset);
                cur = offsetPtr(cur, newTable->recordSize);
            }
            st = offsetPtr(st, 4);
        }
    }    
    
    free(table);
    newTable->database->size += delta;
    *ptable = newTable;
    return 1;
}
#define tableForeach(table, ptr) \
    void* ptr = table->buf; \
    for(int i = 0; i < table->recordNum; i++, ptr = offsetPtr(ptr, table->recordSize))

int tableExpand(SFSTable **ptable, uint32_t addSize){
    uint32_t oldStorSize = (*ptable)->storSize;
    uint32_t newStorSize;
    if(oldStorSize + (*ptable)->freeSpace >= addSize) newStorSize = oldStorSize *2;
    else newStorSize = oldStorSize + addSize;
    return sfsTableReserve(ptable, newStorSize);
}
void* sfsTableAddRecord(SFSTable **ptable){
    uint32_t recordSize = (*ptable)->recordSize;
    if((*ptable)->freeSpace < recordSize) tableExpand(ptable, recordSize);
    SFSTable *table = *ptable;

    void* retPtr = offsetPtr(table->buf, table->recordNum * recordSize);
    
    table->recordNum++;
    table->freeSpace -= table->recordSize;
    
    return retPtr;
}
SFSVarchar* sfsTableAddVarchar(SFSTable **ptable, uint32_t varcharLen, const char* src){
    uint32_t varcharSize = sizeof(SFSVarchar) + varcharLen;
    if((*ptable)->freeSpace < varcharSize) tableExpand(ptable, varcharSize);
    SFSTable *table = *ptable;
    
    SFSVarchar *retPtr = table->lastVarchar;
    retPtr = negOffsetPtr(retPtr, varcharSize);
   
    retPtr->len = varcharLen;
    if(src) memcpy(retPtr->buf, src, varcharLen);
    
    table->lastVarchar = retPtr;
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
        sfsTableRelease(db->table[i]);
    }
    free(db);
}
//model==1 convert ptr to offset, vice versa
void recordVarcharAndPtrConvert(SFSTable *table, void *record, int model){
    const SFSVarchar *recordMeta = table->recordMeta;
    uint32_t *cur = (uint32_t*)record;
    for(int i=0; i < recordMeta->len; i++){
        uint8_t type = (uint8_t)recordMeta->buf[i];
        if(type){
            cur = offsetPtr(cur, type);
        } else {
            if(model){
                *cur = ptrOffset(table, (void*)(*cur));
            } else{
                *cur = (uint32_t)offsetPtr(table, *cur);
            }
            cur = offsetPtr(cur, 4);
        }
    }    
}
void tablePtrToOffsetConvert(SFSTable *table, uint32_t databaseOffset){
    table->database = (SFSDatabase*)databaseOffset;
    table->lastVarchar = (SFSVarchar*)ptrOffset(table, table->lastVarchar);
    table->recordMeta = (SFSVarchar*)ptrOffset(table, table->recordMeta);
    tableForeach(table, record){
        recordVarcharAndPtrConvert(table, record, 1);
    }
}

void tableOffsetToPtrConvert(SFSTable *table, SFSDatabase *db){
    table->database = db;
    table->lastVarchar =  offsetPtr(table, (uint32_t)table->lastVarchar);
    table->recordMeta = offsetPtr(table, (uint32_t)table->recordMeta);
    tableForeach(table, record){
        recordVarcharAndPtrConvert(table, record, 0);
    }
}
int sfsDatabaseSave(char *fileName, SFSDatabase* db){
    if(strlen(fileName) > 4096) {
        sprintf(errmsg, "file name is too long, is .4096%s", fileName);
        return 0;
    }
    FILE *fp = fopen(fileName, "wb");
    if(!fp){
        int errnum = errno;
        sprintf(errmsg, "error in open file: %s, file name is %s",strerror(errnum), fileName);
        return 0;
    }
    uint32_t tableOffset = sizeof(SFSDatabase);
    SFSTable* table[16];
    for(int i=0; i < db->tableNum; i++){
        table[i] = db->table[i];
        tablePtrToOffsetConvert(table[i], tableOffset);
        db->table[i] = (SFSTable*)tableOffset;
        tableOffset += table[i]->size;
    }

    muti_crc32_init();
    muti_crc32_update(&(db->version), sizeof(SFSDatabase));
    for(int i=0; i < db->tableNum; i++) muti_crc32_update(table[i], table[i]->size);
    db->crc = muti_crc32_get();


    fwrite(db, sizeof(SFSDatabase), 1, fp);
    for(int i=0; i < db->tableNum; i++) {
        fwrite(table[i], table[i]->size, 1, fp);
    }
    fclose(fp);

    for(int i=0; i < db->tableNum; i++){
        db->table[i] = table[i];
        tableOffsetToPtrConvert(table[i], db);
    }
    return 1;
}
SFSDatabase* sfsDatabaseCreateLoad(char *fileName){
    if(strlen(fileName) > 4096) {
        sprintf(errmsg, "file name is too long, is .4096%s", fileName);
        return NULL;
    }
    FILE *fp = fopen(fileName, "rb");
    if(!fp){
        int errnum = errno;
        sprintf(errmsg, "error in open file: %s, file name is %s",strerror(errnum), fileName);
        return NULL;    
    }

    
    uint32_t magic = 0;
    fread(&magic, sizeof(uint32_t), 1, fp);
    if(magic!=MAGIC) {
        sprintf(errmsg, "the file %s is not a sfs file", fileName);
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    uint32_t fileSize = ftell(fp);
    
    SFSDatabase * db = malloc(fileSize);
    fseek(fp, 0L, SEEK_SET);
    fread(&db, fileSize, 1, fp);

    uint32_t crc = crc32(&(db->version), fileSize - 2*sizeof(uint32_t));
    if(crc != db->crc){
        sprintf(errmsg, "the file crc is wrong, file name is %s", fileName);
        free(db);
        return NULL;
    }

    for(int i=0; i < db->tableNum; i++){
        db->table[i] = offsetPtr(db, (uint32_t)db->table[i]);
        tableOffsetToPtrConvert(db->table[i], db);
        db->table[i]->database = db;
    }

}

SFSTable* sfsDatabaseAddTable(SFSDatabase *db, uint32_t initStorSize, const SFSVarchar *recordMeta){
    if(initStorSize == 0) initStorSize = 16 * calcRecordSize(recordMeta);
    uint32_t tableSize = calcTableSize(initStorSize, recordMeta);
    SFSTable *table = sfsTableCreate(initStorSize, recordMeta, db);
    db->table[db->tableNum] = table;
    db->tableNum++;
    return table;
}

// return the lastest err
char *sfsErrMsg(){
    return errmsg;  
}


