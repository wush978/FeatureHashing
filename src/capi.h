#ifndef __CAPI_H__
#define __CAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

void FeatureHashing_md5(const char* buf, int size, char retval[16]);
uint32_t FeatureHashing_crc32(const char* buf, int size);

#ifdef __cplusplus
}
#endif

#endif //__CAPI_H__