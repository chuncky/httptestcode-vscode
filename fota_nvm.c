/********************************************************************************//**
 * @file fota_nvm.c
 * @version   V1.00
 * @brief     
 * @note
 * @author   Tanshi Li
 * Copyright (c) 2021 ASR Microelectronics (Shanghai) Co., Ltd. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fota_typedef.h"
#ifdef MINGW64
#include <Windows.h>
#define Fota_printf printf
#else
#include "asr_crane.h"
#include "fota_interface.h"
#include "utilities.h"
#define Fota_printf uart_printf
#endif

#include "fota_nvm.h"
#include "fota_define.h"
#include "fota_hmd5.h"

static fota_char fota_pid[34]={0};
static fota_char fota_psec[34]={0};
static fota_char fota_sign[34]={0};

static fota_char FotaDevice_Registered=1;
static NV_FOTA_VERSION fota_version;

extern UpgradeParamInfo *ru_params;
extern DownParamInfo *rd_params;
extern VersionInfo *fota_info;
extern MobileParamInfo *cv_params;

int is_deviceregistered(void)
{
	if (FotaDevice_Registered==0)
		return 1;
	else 
		return 0;
}

int set_device_registered(void)
{
	FotaDevice_Registered=0;
	return 0;
}


fota_char* fota_get_fota_psec(void)
{
	return (fota_char*)fota_psec;
}

fota_char* fota_get_fota_pid(void)
{
	return (fota_char*)fota_pid;
}










FOTA_BOOL fota_get_reg_info(void)
{

	char *sign = NULL;
	Fota_printf("FOTA-->>> adups: fota_get_reg_info deviceid=%s,psec=%s,mid=%s\r\n",fota_pid,fota_psec,cv_params->mid);
	sign = fota_hmd5_pid_psec_mid(fota_pid,fota_psec,cv_params->mid);
	
	if(strcmp(sign,(char *)fota_sign) == 0)
	{
		Fota_printf("FOTA-->>> adups: fota_pid=%s,fota_psec=%s,sign=%s\r\n",fota_pid,fota_psec,sign);
		return FOTA_TRUE;
	}

	return FOTA_FALSE;

}

FOTA_BOOL fota_set_reg_info(fota_char *devid,fota_char *devsec)
{
	NV_FOTA_REG temp;
	char *sign;
	memset(fota_pid,0,sizeof(fota_pid));
	memset(fota_psec,0,sizeof(fota_psec));
	memset(temp.str,0,sizeof(temp.str));

	if(devid != NULL)
	{
		strcpy((char *)fota_pid, (char *)devid);
		strcpy((char *)temp.str, (char *)devid);
	}
	
	memset(temp.str,0,sizeof(temp.str));
	if(devsec != NULL)
	{
		strcpy((char *)fota_psec, (char *)devsec);
		strcpy((char *)temp.str, (char *)devsec);
	}

	if(devid != NULL && devsec != NULL)
	{
		sign = fota_hmd5_pid_psec_mid(fota_pid,fota_psec,cv_params->mid);
		strcpy((char *)fota_sign, (char *)sign);
		strcpy((char *)temp.str, (char *)sign);
	}
	set_device_registered();
	
	Fota_printf("FOTA-->>> adups: fota_pid=%s,fota_psec=%s,sign=%s\r\n",fota_pid,fota_psec,sign);
	return FOTA_TRUE;
}



FOTA_BOOL fota_get_version(char *version)
{
		strcpy((char*)version, (char*)fota_version.str);
		Fota_printf("FOTA-->>>adups: read version succeed =%s\r\n",fota_version.str);
		return FOTA_TRUE;

}

FOTA_BOOL fota_set_version(char *version)
{
	if(version != NULL)
	{
		memcpy((char*)fota_version.str,(char*)version,sizeof(NV_FOTA_VERSION));
	}

	Fota_printf("FOTA-->>>adups: write version succeed version=%s\r\n",fota_version.str);
	return FOTA_TRUE;
}












FOTA_BOOL fota_get_delta_id(char *delta)
{
#if 0	
	NvCmdErr      nvRet;
	NV_FOTA_DELTA_ID temp;

	memset(&temp, 0, sizeof(temp));
	nvRet = NV_Read(NV_FOTA_CV_DELTAID, (UINT8*)&temp,  sizeof(NV_FOTA_DELTA_ID));
	if(nvRet != NV_ERR_OK)
	{
		Fota_printf("FOTA-->>> adups: read delta id failed nvRet=%d\r\n",nvRet);
		
		return FOTA_FALSE;
	}
	else
	{
		//Fota_printf("FOTA-->>> adups: read delta id succeed.\r\n");
		strcpy((char*)delta, (char*)temp.str);
		return FOTA_TRUE;
	}
#endif	
	return FOTA_TRUE;
}




