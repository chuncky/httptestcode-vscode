/**************************************************************************//**
 * @file     fotadownload.c
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
#ifdef MINGW64
#include "mingwtype.h"
#include <stdio.h>
#else
#include "asr_crane.h"
#include "msfota.h"
#include "mbedtls/md5.h"
#include "mbedtls/compat-1.3.h" //mbedtls
#include "ip.h"
#include "fota_nvm.h"
#include "fota_cjson.h"
#include "fota_hmd5.h"
#include "fota_interface.h"
#include "fotahttp_typedef.h"
#include "msfotadm_fbf_parse.h"
#include "msfotadm.h"
#endif

#include "fota_typedef.h"
#include "fota_stdlib.h"
#include "fota_http_parser.h"
#include "fota_http.h"
#include "fota_md5.h"
#include "fota_config.h"
#ifdef MINGW64
#include <winsock2.h>
#define Fota_printf printf
#else
#include <sys/socket.h>
#define Fota_printf uart_printf
#endif
#include "platformbase.h"
#include "fota_define.h"

static MobileParamInfo params;
MobileParamInfo *cv_params=&params;
static UpgradeParamInfo RuData;
UpgradeParamInfo *ru_params=&RuData;
static DownParamInfo gv_adups_RdData;
DownParamInfo *rd_params=&gv_adups_RdData;
static VersionInfo version;
VersionInfo *fota_info = &version;
extern MD5_CTX adups_md5;
extern UINT8 * fota_tcp_server_ip;


#ifdef REPORT_DL
fota_char fota_report_dl[384] = {0};
#endif


MobileParamInfo * fota_get_cv_params(void)
{
	return cv_params;
}

UpgradeParamInfo * fota_get_ru_params(void)
{
	return ru_params;
}

DownParamInfo * fota_get_rd_params(void)
{
	return rd_params;
}


VersionInfo * fota_get_fota_info(void)
{
	return fota_info;
}




fota_char* fota_get_report_dl(void)
{
	return (fota_char*)fota_report_dl;
}




void FotaParams_init(void)
{

	static char delta_id[15];
	FOTA_BOOL res;
	
	fota_memset(&params, 0, sizeof(params));
	fota_memset(&version, 0, sizeof(version));
    params.mid = fota_get_mid();
	params.version =  fota_get_device_version();
    params.oem = fota_get_device_oem();
    params.models = fota_get_device_model();
    params.productid= fota_get_device_product_id();
    params.productsec= fota_get_device_product_sec();
    params.platform = fota_get_device_platform() ;
    params.device_type = fota_get_device_type();

	

	ru_params->mid = fota_get_mid();
	res = fota_get_delta_id((char *) delta_id);
	if(res == FOTA_TRUE && strlen(delta_id) >= 2)
	{
		ru_params->deltaID = delta_id;
	}
	else
	{
		ru_params->deltaID = NULL;
	}
    ru_params->productid= fota_get_device_product_id();
    ru_params->productsec= fota_get_device_product_sec();
    
    
    Fota_MD5_Init(&adups_md5);
    //gadups_retry_num = 0;
	
	//adups_socket_mgr.fota_read_notify=fota_soc_recv;
	//adups_socket_mgr.fota_write_notify=fota_soc_send;
	//adups_socket_mgr.fota_close_notify=fota_notify_soc_close;
	//adups_socket_mgr.fota_net_error_notify=fota_notify_soc_net_error_notify;
}

















