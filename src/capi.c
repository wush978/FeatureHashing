#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "crc32.h"
#include "capi.h"

uint32_t FeatureHashing_crc32(const char* buf, int size) {
  return Crc32_ComputeBuf(0, buf, size);
}

char FeatureHashing_xor(const char* buf, int size) {
  char retval = 0;
  int i;
  if (size > 0) retval = buf[0];
  for(i = 1;i < size;i++) retval ^= buf[i];
  return retval;
}