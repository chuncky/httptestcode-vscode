
#ifndef _PLATFORMBASE_H
#define _PLATFORMBASE_H

#ifdef __cplusplus
extern "C" {
#endif




char * get_platform_current_version(void);

void* platform_memalloc(fota_int32 size);
void platform_memfree(void *p);

void PlatTaskSleep(unsigned second);
FOTA_BOOL fota_get_reg_info(void);
fota_uint32 FotaGetUpdateResult(void);
FOTA_BOOL fota_get_delta_id(char *delta);

#ifdef __cplusplus
}
#endif

#endif
