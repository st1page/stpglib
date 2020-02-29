#ifndef SFS_H
#define SFS_H

// simple file storage
#include <stdint.h>

typedef struct SFSVarchar{
    uint32_t len; /* length of the varchar string */
    char buf[];
}SFSVarchar;

typedef struct SFSTableHdr{
    uint32_t size;              /* size of the table */
    uint32_t storSize;         /* free space left in the table */
    uint32_t freeSpace;         /* free space left in the table */
    uint32_t varcharNum;        /* number of varchars in the table */
    uint32_t recordNum;         /* number of record in the table */
    uint32_t recordSize;        /* size of a record */
    struct SFSVarchar *recordMeta;
    struct SFSVarchar *lastVarchar;
    struct SFSDatabase *database;
    char buf[];
}SFSTableHdr;

typedef struct SFSDatabase{
    uint32_t magic;     /* sfs magic number */
    uint32_t crc;       /* CRC32 checksum of the file */
    uint32_t version;   /* sfs version number of the file */
    uint32_t size;      /* size of the file */
    uint8_t tableNum;   /* number of tables int the file (not more than 16)*/
    uint8_t pad[3];     /* reserved */
    SFSTableHdr *table[16]; /* ptr/offset of tables */
    char buf[];
}SFSDatabase;

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
inline void sfsCompileTest(){
    BUILD_BUG_ON(__SIZEOF_POINTER__ != 4);
    BUILD_BUG_ON(sizeof(SFSVarchar) != 4);
    BUILD_BUG_ON(sizeof(SFSTableHdr) != 36);
    BUILD_BUG_ON(sizeof(SFSDatabase) != 84);
}

int sfsVarcharCons(SFSVarchar *varchar, const char* src);
SFSVarchar* sfsVarcharCreate(uint32_t varcharSize, const char* src);
int sfsVarcharRelease(SFSVarchar *varchar);

int sfsTableCons(SFSTableHdr *table, uint32_t initStorSize, const SFSVarchar *recordMeta, SFSDatabase *db);
SFSTableHdr* sfsTableCreate(uint32_t initStorSize, const SFSVarchar *recordMeta, SFSDatabase *db);
int sfsTableRelease(SFSTableHdr *table);
int sfsTableReserve(SFSTableHdr **table, uint32_t storSize);

void* sfsTableAddRecord(SFSTableHdr **ptable);
SFSVarchar* sfsTableAddVarchar(SFSTableHdr **ptable, uint32_t varcharLen, const char* src);

SFSDatabase* sfsDatabaseCreate(uint32_t storSize);
void sfsDatabaseRelease(SFSDatabase* db);
int sfsDatabaseSave(char *fileName, SFSDatabase* db);
SFSDatabase* sfsDatabaseCreateLoad(char *fileName);
SFSTableHdr* sfsDatabaseAddTable(SFSDatabase *db, uint32_t storSize, const SFSVarchar *recordMeta);


// return the last err
char *sfsErrMsg(); 

#endif

