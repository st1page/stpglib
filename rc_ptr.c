#include "rc_ptr.h"

#include <stdlib.h>

typedef struct RCPtrHdr{
    unsigned int retainCount;
    char buf[];
}RCPtrHdr;


void *rcMalloc(size_t size){
    RCPtrHdr *hdr = (RCPtrHdr *)malloc(sizeof(RCPtrHdr) + size);
    hdr->retainCount = 1;
    return (void*)hdr->buf;
}
void rcRetain(void *ptr){
    RCPtrHdr *hdr = (RCPtrHdr *)( (char*)ptr - sizeof(RCPtrHdr));
    hdr->retainCount++;
}
void rcFree(void *ptr){
    RCPtrHdr *hdr = (RCPtrHdr *)( (char*)ptr - sizeof(RCPtrHdr));
    hdr->retainCount--;
    if(!hdr->retainCount) free(hdr);
}