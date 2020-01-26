#ifndef RC_PTR_H
#define RC_PTR_H

#include <stddef.h>
#define RC(T) T
#define RCPtr(T) T*
extern void *rcMalloc(size_t size);
extern void rcRetain(void *ptr);
extern void rcFree(void *ptr);
extern size_t rcSize(void *ptr);
extern size_t rcUsedMem();
#endif //RC_PTR_H
