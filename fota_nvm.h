/********************************************************************************//**
 * @file fota_nvm.h
 * @version   V1.00
 * @brief     
 * @note
 * @author   Tanshi Li
 * Copyright (c) 2019 ASR Microelectronics (Shanghai) Co., Ltd. All rights reserved.
 */

#ifndef _FOTA_NVM_H_
#define _FOTA_NVM_H_
#include <string.h>
/** include files **/
//#include "bsp_config.h"
//#include "global_types.h"

#define FOTA_PARAMLENTH   (1024)


#define MAX_CALI_DATA_LEN  120
typedef struct
{
    UINT8 str[20];
}NVCatVer;


typedef struct
{
    UINT8 str[20];
}NvVendorCode;

typedef struct
{
    UINT8 calibration_data[MAX_CALI_DATA_LEN];
}NV_QZS_Sensor;

typedef struct
{
    UINT8 str[34];
}NV_FOTA_REG;

typedef struct
{
    UINT8 str[60];
}NV_FOTA_VERSION;

typedef struct
{
    UINT8 str[20];
}NV_FOTA_DELTA_ID;


typedef struct
{
    UINT32 num;
}NV_FOTA_BREAK;


typedef struct NvFotaItemTag{
	                          char fota_param[FOTA_PARAMLENTH];
							  int device_registered;
							  
    /* NV_VENDOR_VER_I              */ NVCatVer      ver;
    /* NV_VENDOR_CODE_I             */ NvVendorCode  code;
    
    /**
     * Your NV are defined here
     */
    /* NV_VENDOR_QZS_SENSOR_I       */ 	NV_QZS_Sensor qzs_sensor;
  
    /* NV_FOTA_REG_DEV_ID       */ 	    NV_FOTA_REG  device_id;
    /* NV_FOTA_REG_DEV_SEC       */ 	NV_FOTA_REG  device_sec;
    /* NV_FOTA_CV_VERSION       */ 	    NV_FOTA_VERSION  version;
    /* NV_FOTA_CV_DELTAID       */ 	    NV_FOTA_DELTA_ID  deltaid;
    /* NV_FOTA_BREAK_POINT       */ 	NV_FOTA_BREAK  break_point;
     /*NV_FOTA_REG_DEV_SIGN 	*/		NV_FOTA_REG  device_sign;

}NvFotaItemTag_t;



FOTA_BOOL fota_get_reg_info(void);
FOTA_BOOL fota_set_reg_info(fota_char *devid,fota_char *devsec);


#endif
