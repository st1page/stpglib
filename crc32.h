#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>

uint32_t crc32(const unsigned char *buf, uint32_t len);

void muti_crc32_init();
void muti_crc32_update(const unsigned char *buf, uint32_t len);
uint32_t muti_crc32_get();
#endif 
