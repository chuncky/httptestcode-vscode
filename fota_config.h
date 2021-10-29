#ifndef _FOTA_CONFIG_H_
#define _FOTA_CONFIG_H_


fota_char* fota_get_device_oem(void);


fota_char* fota_get_device_model(void);


fota_char* fota_get_device_product_id(void);


fota_char* fota_get_device_product_sec(void);


fota_char* fota_get_device_type(void);


fota_char* fota_get_device_platform(void);


fota_char* fota_get_device_network_type(void);


fota_char* fota_get_device_version(void);


fota_char* fota_get_mid(void);


fota_int32 Fota_GetDataPartSize(void);


//fota_uint32 fota_get_block_size(void);
fota_uint32 fota_gettime(void);
#endif

