/**************************************************************************//**
 * @file     platformbase.c
 * @version  V1.00
 * @brief    Fota Download  Source File
 *
 * @note
 * Copyright (C) 2021 ASR Microelectronics, Ltd. Co. All rights reserved.
 *
 ******************************************************************************/
/**
 * @note: we pre-define some tasks , in which users can add
 * code to use.  meanwhile, user can modify these task name by define
 * another name using MACRO if does't like it.
 * additionly, we pre-define the stack size of every task, according to which
 * user may choose own task properly.
 *
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
#define Fota_printf uart_printf
#endif

#define VERSIONLENTH (200)

char current_soft_version[VERSIONLENTH]="1.0";




char * get_platform_current_version(void)
{

	Fota_printf("%s,current_version:%s\r\n",__func__,current_soft_version);

	return current_soft_version;
}

void* platform_memalloc(fota_int32 size)
{
	void *ptr=NULL;
	ptr = malloc(size);
	//Fota_printf("%s,addr=0x%x,size=0x%x\r\n",__func__,ptr,size);
	return ptr;
}


void platform_memfree(void *p)
{
	if(p != NULL)
	{
		//Fota_printf("%s,addr=0x%x\r\n",__func__,p);
        free(p);
		p = NULL;
	}
}


void PlatTaskSleep(unsigned second)
{


    Sleep(second);

}




fota_uint32 FotaGetUpdateResult(void)
{
// 0 success; 1 failed; 2 else
	FOTA_BOOL ret;
	static char version[50];
	UINT8 data[5];
#if 0
	MsfotadmReadBackup((UINT8 *)data,5);
	Fota_printf("Adups_get_update_result:%d %d %d %d %d",data[0],data[1],data[2],data[3],data[4]);
	if(data[0] == 'A' && data[1] == 'D' && data[2] == 'U' && data[3] == 'P' && data[4] == 'S')
	{
		Fota_printf("%s:have break point!\r\n",__FUNCTION__);
		return 2; //no update event need report
	}
	ret = fota_get_version(version);

	if(ret == FOTA_FALSE || strlen(version) <= 2)
	{
		Fota_printf("%s:no update event!\r\n",__FUNCTION__);
		return 2; //no update event
	}

	if(strcmp(fota_get_device_version(), version) == 0)
	{
		Fota_printf("%s: update failed!\r\n",__FUNCTION__);
		return 1; //failed
	}
	else
	{
		Fota_printf("%s: update succeed!\r\n",__FUNCTION__);
		return 0; //succeed
	}
#endif
	return 0;

}






































