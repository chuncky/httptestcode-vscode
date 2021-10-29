
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef MINGW64
#else
#include "asr_crane.h"
#endif
#include "stdarg.h"
#include "math.h"
#include "fota_typedef.h"
#include "fota_stdlib.h"

#include "platformbase.h"



#define  FOTA_DT_UTC_BASE_YEAR  1970
#define  FOTA_DT_MONTH_PER_YEAR    12
#define  FOTA_DT_DAY_PER_YEAR         365
#define  FOTA_DT_SEC_PER_DAY    86400
#define  FOTA_DT_SEC_PER_HOUR      3600
#define  FOTA_DT_SEC_PER_MIN    60
const fota_uint8 s_fota_dt_day_per_mon[FOTA_DT_MONTH_PER_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

fota_int32 fota_sprintf(fota_char * buffer, const fota_char *format, ...)
{
	fota_int32 iReturn;
	va_list Args;
	va_start(Args,format);
	iReturn = vsprintf(buffer,format,Args);
	va_end(Args);
	return iReturn;
}


fota_int32 fota_snprintf(fota_char * buffer, fota_int32 size,const fota_char *format, ...)
{
	fota_int32 iReturn;
	va_list Args;
	va_start(Args,format);
#ifndef WIN32	
	iReturn = vsnprintf(buffer,size,format,Args);
#else
	iReturn = _vsnprintf(buffer,size,format,Args);
#endif
	va_end(Args);
	return iReturn;
}


void * fota_memcpy(void * dest, const void * src, fota_uint32 count)
{
   if((dest!=NULL)&&(src!=NULL)&&(count!=0))
	return memcpy(dest,src,count);
   else
   	return NULL;
}

void * fota_memset(void * dest, char c, fota_uint32 count )
{
   if((dest!=NULL)&&(count!=0))
	return memset(dest,c,count);
   else
   	return NULL;
}

fota_int32 fota_memcmp(void * buf1, void * buf2, fota_uint32 count)
{
   if((buf1!=NULL)&&(buf2!=NULL)&&(count!=0))
	return memcmp((const void *)buf1,(const void *)buf2,count);
   else
   	return 0;
}


fota_char * fota_strcat(fota_char * strDestination, const fota_char * strSource )
{
   if((strDestination!=NULL)&&(strSource!=NULL))
	return strcat(strDestination,strSource);
   else
   	return NULL;
}


fota_char * fota_strncat(fota_char * strDestination, const fota_char * strSource,fota_uint32 len)
{
   if((strDestination!=NULL)&&(strSource!=NULL)&&(len!=0))
	return strncat(strDestination,strSource,len);
   else
   	return NULL;
}


fota_uint32 fota_strlen(const fota_char * str)
{
   if(str!=NULL)
	return (fota_uint32)strlen(str);
   else
   	return 0;
}

fota_int32 fota_strncmp(const fota_char * string1, fota_char * string2, fota_uint32 count)
{
	return strncmp(string1,string2,count);

}

fota_int32 fota_strcmp(const fota_char * string1, const fota_char * string2 )
{
	return strcmp(string1,string2); 	
}

fota_int32 fota_wstrcmp(const fota_wchar * string1, const fota_wchar * string2)
{
    fota_int32 ret = 0 ;
    
    if( string1 == NULL && string2 == NULL )
    {
        return 0;
    }
    else if( string1 == NULL && string2 != NULL )
    {
        return -1;
    }
    else if( string1 != NULL && string2 == NULL )
    {
        return 1;
    }
    else
    {
/*lint -save -e613 */
        ret = *(fota_wchar *)string1 - *(fota_wchar *)string2;/*lint !e613 */

        while( (!ret) && *string2 && *string1)/*lint !e613 */
        {
            ++string1;/*lint !e613 */
            ++string2;
            ret = *(fota_wchar *)string1 - *(fota_wchar *)string2;
        }
/*lint -restore */
        return ( ret > 0 ? 1 : ( ret < 0 ? -1 : 0 ) ); 
    }
}


fota_char * fota_strcpy(fota_char * strDestination, const fota_char * strSource)
{
   if((strDestination!=NULL)&&(strSource!=NULL))
	return strcpy(strDestination, strSource);
   else
   	return NULL;
}

fota_char * fota_strncpy(fota_char * strDest, const fota_char * strSource, fota_uint32 count)
{
   if((strDest!=NULL)&&(strSource!=NULL)&&(count!=0))
	return strncpy(strDest,strSource,count);
   else
   	return NULL;
}


fota_char * fota_strstr(const fota_char * str, const fota_char * strSearch)
{
   if(str!=NULL)
	return strstr(str,strSearch);
   else
   	return NULL;
}

fota_char * fota_strchr(const fota_char * str, fota_int32 ch)
{
   if(str!=NULL)
	return strchr(str,ch);
   else
   	return NULL;
}



void fota_itoa(fota_uint32 i, fota_char *buf, fota_uint8 base)
{
	fota_char *s;
	#define FOTA_ITOA_LEN	20
	fota_int32 rem;
	static fota_char rev[FOTA_ITOA_LEN+1];

	rev[FOTA_ITOA_LEN] = 0;
	if (i == 0)
		{
		buf[0] = '0';
		++(buf);
		return;
		}
	s = &rev[FOTA_ITOA_LEN];
	while (i)
		{
		rem = i % base;
		if (rem < 10)
			*--s = rem + '0';
		else if (base == 16)
			*--s = "abcdef"[rem - 10];
		i /= base;
		}
	while (*s)
		{
		buf[0] = *s++;
		++(buf);
		}
}

fota_uint32 fota_atoi(fota_char * str)
{
 	if(str!=NULL)
		return (fota_uint32)atoi(str);
	else
		return 0;
}

void* fota_memalloc(fota_int32 size)
{
	void *ptr=NULL;
	
	ptr = platform_memalloc(size);
	return ptr;
}

void fota_memfree(void *p)
{
	platform_memfree(p);
}

