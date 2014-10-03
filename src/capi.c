#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "crc32.h"
#include "capi.h"

uint32_t FeatureHashing_crc32(const char* buf, int size) {
  return Crc32_ComputeBuf(0, buf, size);
}
