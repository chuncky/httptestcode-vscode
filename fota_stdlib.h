#ifndef _FOTA_STDLIB_H_
#define _FOTA_STDLIB_H_
#include "fota_typedef.h"

fota_int32 fota_sprintf(fota_char * buffer, const fota_char *format, ...);
fotacomm_extern fota_int32 fota_snprintf(fota_char * buffer, fota_int32 size,const fota_char *format, ...);
fotacomm_extern void * fota_memcpy(void * dest, const void * src, fota_uint32 count);
fotacomm_extern void * fota_memset(void * dest, char c, fota_uint32 count );
fotacomm_extern fota_int32 fota_memcmp(void * buf1, void * buf2, fota_uint32 count);
fotacomm_extern fota_char * fota_strcat(fota_char * strDestination, const fota_char * strSource );
fotacomm_extern fota_char * fota_strncat(fota_char * strDestination, const fota_char * strSource,fota_uint32 len);
fotacomm_extern fota_uint32 fota_strlen(const fota_char * str);
fotacomm_extern fota_int32 fota_strncmp(const fota_char * string1, fota_char * string2, fota_uint32 count);
fotacomm_extern fota_int32 fota_strcmp(const fota_char * string1, const fota_char * string2 );
fotacomm_extern fota_int32 fota_wstrcmp(const fota_wchar * string1, const fota_wchar * string2);
fotacomm_extern fota_char * fota_strcpy(fota_char * strDestination, const fota_char * strSource);
fotacomm_extern fota_char * fota_strncpy(fota_char * strDest, const fota_char * strSource, fota_uint32 count);
fotacomm_extern fota_char * fota_strstr(const fota_char * str, const fota_char * strSearch);
fotacomm_extern fota_char * fota_strchr(const fota_char * str, fota_int32 ch);
fotacomm_extern void fota_itoa(fota_uint32 i, fota_char *buf, fota_uint8 base);
fotacomm_extern fota_uint32 fota_atoi(fota_char * str);
fotacomm_extern void* fota_memalloc(fota_int32 size);
fotacomm_extern void fota_memfree(void *p);
#endif
