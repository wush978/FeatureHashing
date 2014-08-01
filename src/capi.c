#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "md5.h"
#include "crc32.h"
#include "capi.h"

void md5(const char* buf, int size, char retval[16]) {
  MD5_CTX ctx;
  MD5Init(&ctx);
  MD5Update(&ctx, buf, size);
  MD5Final(retval, &ctx);
}

uint32_t crc32(const char* buf, int size) {
  return Crc32_ComputeBuf(0, buf, size);
}
