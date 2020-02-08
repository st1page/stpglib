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
    uint32_t freeSpace;         /* free space left in the table */
    uint32_t varcharNum;        /* number of varchars in the table */
    uint32_t lastVarcharOffset; /* offset of the latest inserted varchar */
    uint32_t recordNum;         /* number of record in the table */
    uint32_t recordSize;        /* size of a record */
    uint32_t recordMetaOffset;  /* offset of the fields info (a varchar)*/
    char buf[];
}SFSTableHdr;

typedef struct SFSFileHdr{
    uint32_t magic;     /* sfs magic number */
    uint32_t version;    /* sfs version number of the file */
    uint32_t crc;       /* CRC32 checksum of the file */
    uint32_t size;      /* size of the file */
    uint8_t pad2[4];    /* reserved */
    uint8_t tableNum;   /* number of tables int the file (not more than 16)*/
    uint8_t pad3[3];    /* reserved */
    uint32_t tableOffset[16]; /* offset of tables */
    char buf[];
}SFSFileHdr;

int sfsVarcharCons(SFSVarchar *varchar, const char* src);
SFSVarchar* sfsVarcharCreate(uint32_t varcharSize, const char* src);
int sfsVarcharRelease(SFSVarchar *varchar);

int sfsTablewCons(SFSTableHdr *table, uint32_t storSize, SFSVarchar *recordMeta);
void* sfsTableAddRecord(SFSTableHdr *table);
SFSVarchar* sfsTableAddVarchar(SFSTableHdr *table, uint32_t varcharLen, const char* src);
uint32_t sfsTableVarcharOffset(SFSTableHdr *table, SFSVarchar *varchar);
SFSTableHdr* sfsFileAddTable(SFSFileHdr *file, uint32_t storSize, SFSVarchar *recordMeta);
SFSFileHdr* sfsFileCreate(uint32_t storSize);
SFSFileHdr* sfsFileLoad(char *fileName);
void sfsFileRelease(SFSFileHdr* sfsFile);
void sfsFileSave(char *fileName, SFSFileHdr* sfsFile);

// return the last err
char *sfsErrMsg(); 

#endif

