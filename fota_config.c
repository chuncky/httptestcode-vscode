
#include "fota_typedef.h"
#include "fota_define.h"
#if !defined(MINGW64)
#include "asr_crane.h"
#include "IMEI.h"
#include "fotacomm.h"
#endif
#include "platformbase.h"

fota_char* fota_get_device_oem(void)
{
	return FOTA_SERVICE_OEM;	
}

fota_char* fota_get_device_model(void)
{
	return FOTA_SERVICE_MODEL;		
}

fota_char* fota_get_device_product_id(void)
{
	return FOTA_SERVICE_PRODUCT_ID;		
}

fota_char* fota_get_device_product_sec(void)
{
	return FOTA_SERVICE_PRODUCT_SEC;		
}

fota_char* fota_get_device_type(void)
{
	return FOTA_SERVICE_DEVICE_TYPE;		
}

fota_char* fota_get_device_platform(void)
{
	return FOTA_SERVICE_PLATFORM;		
}

fota_char* fota_get_device_network_type(void)
{
	return DEVICE_NETWORKTYPE;		
}

fota_char* fota_get_device_version(void)
{
	return get_platform_current_version();
}
//IMEIStr imeiStr;
char *imeiStr="001068000000006";

fota_char* fota_get_mid(void)
{
	//IMEIReadStr(&imeiStr);
	//uart_printf("IMEI#=%s\r\n",imeiStr.imei_digits);
	return imeiStr;
}

fota_int32 Fota_GetDataPartSize(void)
{
	return 0x500000;

}

fota_uint32 fota_gettime(void)
{
    	//return (PMIC_RTC_GetTime_Count(SYS_OFFSET) - GetTimeZone()*60);
		return 112;
}

