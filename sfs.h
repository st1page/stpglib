#ifndef SFS_H
#define SFS_H

// simple file storage
#include <stdint.h>

typedef struct SFSVarchar{
    uint32_t size;
    char buf[];
}SFSVarchar;

typedef struct SFSFileHdr{
    uint16_t magic;
    uint8_t version;
    uint8_t pad1[1];
    uint32_t crc;
    uint32_t size;
    uint8_t pad2[4];
    uint8_t tableNum;
    uint8_t pad3[3];
    uint32_t tableOffset[16];
}SFSFileHdr;

typedef struct SFSTableHdr{
    uint32_t size;
    uint32_t freeSpace;
    uint32_t varcharNum;
    uint32_t lastVarcharOffset;
    uint32_t recordNum;
    uint32_t recordMetaOffset;
}SFSTableHdr;


void sfsVarcharNew(char *dst, uint32_t size, char* src);

void sfsTablewNew(char *dst, uint32_t size, uint32_t fieldNum, uint8_t* fields);
void* sfsTableNewRecord(SFSTableHdr *table);
// return the offset measured from the start of table 
uint32_t sfsTableNewVarchar(SFSTableHdr *table, uint32_t size);

SFSTableHdr* sfsFileNewTable(SFSFileHdr *file);
SFSFileHdr* sfsFileCreate(uint32_t size);
SFSFileHdr* sfsFileRelease();
SFSFileHdr* sfsFileLoad(char *fileName);
SFSFileHdr* sfsFileSave(char *fileName);

char *sfsErrMsg();

#endif

