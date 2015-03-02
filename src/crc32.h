#ifndef _CRC32_H_
#define _CRC32_H_

bool crc32file(const char * const name, unsigned int* crc, long* charcnt);

unsigned int crc32buf(char* buf, size_t len);

#endif //_CRC32_H_
