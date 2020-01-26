#include "rc_ptr.h"

#include <stdlib.h>

static size_t usedMem = 0;
typedef struct RCPtrHdr{
    unsigned int retainCount;
    size_t size;
    char buf[];
}RCPtrHdr;

#define getHdr(x) ((RCPtrHdr *)( (char*)x - sizeof(RCPtrHdr)))
void *rcMalloc(size_t size){
    RCPtrHdr *hdr = (RCPtrHdr *)malloc(sizeof(RCPtrHdr) + size);
    hdr->retainCount = 1;
    hdr->size = size;
    usedMem += size;
    return (void*)hdr->buf;
}
void rcRetain(void *ptr){
    RCPtrHdr *hdr = getHdr(ptr);
    hdr->retainCount++;
}
void rcFree(void *ptr){
    RCPtrHdr *hdr = getHdr(ptr);
    hdr->retainCount--;
    if(!hdr->retainCount) {
        free(hdr);
        usedMem -= hdr->size;
    }
}
extern size_t rcSize(void *ptr){
    RCPtrHdr *hdr = getHdr(ptr);
    return hdr->size;
}
extern size_t rcUsedMem(){
    return usedMem;
}