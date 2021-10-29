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
#include "fota_main.h"
#include "fota_define.h"
#include "fota_nvm.h"

volatile UINT8  fota_tcp_server_ip[] = "211.144.200.173";
UINT16  fota_tcp_server_port = 8080;
http_parser_settings settings;
http_parser parser;

#define FOTA_RX_TRUNK_BUF_LEN		64
#define FOTA_RX_TRUNK_NODE_LEN		7
#define FOTA_NET_RETRY_MAXCNT 		5

static fota_uint8 s_fota_rx_trunk_buf[FOTA_RX_TRUNK_BUF_LEN]={0};
static fota_uint32 s_fota_rx_trunk_buf_len=0;

extern MobileParamInfo *cv_params;
extern UpgradeParamInfo *ru_params;
extern DownParamInfo *rd_params;
extern VersionInfo *fota_info;
extern http_parser *s_parser;
FotaState fota_state = STATE_INIT;
fota_int32 total_download = 0;
static fota_int32 outfile;

static fota_uint8 fota_retry_num = 0;


FOTA_BOOL fota_runing = FOTA_FALSE;

MD5_CTX adups_md5;
UINT16 FOTA_DOWNLOAD_PORT = 80;
fota_char *DOWNLOAD_DELTA_URL=NULL;
fota_char *FOTA_DOWNLOAD_DOMIAN=NULL;
fota_int32 fotadownload_result = 0;
fota_char downloadurl[]="http://211.144.200.173:8080/dl/crane/delta1.bin";

#define FILE_NAME_LEN 256
char file_nameforsave[FILE_NAME_LEN] = "web.bin";
FILE *file_web;



void fotadevice_checkversion(void);

FotaState fota_get_fota_state(void)
{
	return fota_state;
}

UINT8* fota_get_tcp_server_ip(void)
{
	return (UINT8*)fota_tcp_server_ip;
}





int FotaDL_DataRead(fota_int32 offset,fota_int32 len,fota_uint8 *buf)
{
	int ret=0;
	Fota_printf("%s,addr:0x%x,len:0x%x\r\n",__func__,offset,len);
	fseek(file_web,  offset, SEEK_SET);
	fread(buf, 1, len, file_web);
	return ret;
}

int FotaDL_DataWrite(fota_int32 offset,fota_int32 len,fota_uint8 *buf)
{
	int ret=0;
	
	Fota_printf("%s,addr:0x%x,len:0x%x\r\n",__func__,offset,len);
	fseek(file_web,  offset, SEEK_SET);
	fwrite(buf, 1, len, file_web);

	return 0;
}

/**
 * @brief    Fota state report to UI app.
 *
 * @param[in]  state  current fota state
 *
 * @return   None
 *
 * @details  This function is used to report the fota state the UI. 
 *           
 */

void FotaReport_CurrState(FotaState state)
{
	if(state == STATE_CV)
	{
	}
	
}



void FotaReport_CurrDownloadPercent(fota_int32 percent)  //0-100
{
	//..	
	static INT32 oldpercent;
	if(percent!=oldpercent){
		Fota_printf("%s,percent=%d\r\n",__FUNCTION__, percent);
	}
	oldpercent=percent;
}


void fota_deinit(void)
{
	if(fota_state == STATE_DL)
	{
		Fota_printf(  "total lendth is %d fota_info->fileSize length is %d\r\n",total_download,fota_info->fileSize);
		rd_params->download_status = DL_NETWORK_ERROR;
		fota_info->download_status= DL_NETWORK_ERROR;
		Fota_printf(  "adups_fota_deinit file download length is not enough!\r\n");

	}
}

void fota_ram_clean(void)
{
	Fota_printf("FOTA-->>>\r\n fota_ram_clean\r\n");
	if(fota_info && (fota_info->msg != NULL))
	{
		fota_memfree(fota_info->msg);
		fota_info->msg = NULL;
	}
	if(fota_info && (fota_info->versionName != NULL))
	{
		fota_memfree(fota_info->versionName);
		fota_info->versionName = NULL;
	}
	if(fota_info && (fota_info->md5sum != NULL))
	{
		fota_memfree(fota_info->md5sum);
		fota_info->md5sum = NULL;
	}
	if(fota_info && (fota_info->deltaID != NULL))
	{
		fota_memfree(fota_info->deltaID);
		fota_info->deltaID = NULL;
	}
	if(fota_info && (fota_info->deltaUrl != NULL))
	{
		fota_memfree(fota_info->deltaUrl);
		fota_info->deltaUrl = NULL;
	}
	if(fota_info && (fota_info->publishDate != NULL))
	{
		fota_memfree(fota_info->publishDate);
		fota_info->publishDate = NULL;
	}
	if(fota_info && (fota_info->releaseContent != NULL))
	{
		fota_memfree(fota_info->releaseContent);
		fota_info->releaseContent = NULL;
	}
	if(FOTA_DOWNLOAD_DOMIAN)
	{
		fota_memfree(FOTA_DOWNLOAD_DOMIAN);
		FOTA_DOWNLOAD_DOMIAN = NULL;
	}

	if(DOWNLOAD_DELTA_URL)
	{
		fota_memfree(DOWNLOAD_DELTA_URL);
		DOWNLOAD_DELTA_URL = NULL;
	}
	
	fota_memset(s_fota_rx_trunk_buf, 0, FOTA_RX_TRUNK_BUF_LEN);
	s_fota_rx_trunk_buf_len = 0;
}


/**
 * @brief    Send Fota device Register to the server.
 *
 * @param    None
 *
 * @return   None
 *
 * @details  This function is used to send fota device register to the server. 
 *           and must be called before start to get new deviceversion.
 */
void FotaError_Progress(void)
{
	Fota_printf("%s-01\r\n" , __func__);



}

void FotaMain_Progress(VersionInfo *info, fota_int32 state)
{
	int packagesize,is_dfota;
	fota_char deltaid[50] ={0};
	fota_char fota_md5_result[16]={0};
	fota_char fota_md5_info[33]={0};
	fota_int32 ret=0;

	Fota_printf("FOTA-->>>\r\nmain_callback status=%d, msg=%s, versionName=%s, fileSize=%d\r\n" , info->status, (info->msg == NULL? "NULL":info->msg), (info->versionName == NULL? "NULL":info->versionName), info->fileSize);
	Fota_printf("FOTA-->>>\r\ndeltaID=%s, md5sum=%s, publishDate=%s\r\n" , (info->deltaID == NULL? "NULL":info->deltaID), (info->md5sum == NULL? "NULL":info->md5sum), (info->publishDate == NULL? "NULL":info->publishDate));
	Fota_printf("FOTA-->>>\r\nreleaseContent=%s\r\n" , (info->releaseContent == NULL? "NULL":info->releaseContent));
	Fota_printf("FOTA-->>>\r\nstorageSize=%d, battery=%d, download_status=%d\r\n", info->storageSize, info->battery, info->download_status);
	Fota_printf("FOTA-->>>\r\ndownload_start=%d, download_end=%d, update_status=%d\r\n" ,info->download_start, info->download_end, info->update_status);
	Fota_printf("FOTA-->>>\r\ndeltaUrl=%s\r\n" , (info->deltaUrl == NULL? "NULL":info->deltaUrl));
	Fota_printf("FOTA-->>>state=%d\r\n" , state);

	switch(state)
	{
		case STATE_RG:
			if(info->download_status == DL_SUCCESS && info->status==FOTA_SUCCESS)
			{
				Fota_printf("FOTA-->>>\r\nadups_main_progress rg succeed\r\n");
				//ret=adupsWriteRegFile(adups_get_login_file_path(),fota_info->deviceId,fota_info->deviceSecret);
				ret=fota_set_reg_info(fota_info->deviceId,fota_info->deviceSecret);
				if(ret == FOTA_FALSE)
				{
					Fota_printf("FOTA-->>>\r\nwrite reg file fail close fota\r\n");
					fota_retry_num = FOTA_NET_RETRY_MAXCNT;
					FotaError_Progress();
					
				}
				else
				{
					//if(0==adups_get_pid_psec(adups_get_login_file_path(), fota_pid, fota_psec,cv_params->mid))
					if(fota_get_reg_info() == FOTA_TRUE)
					{
						fota_deinit();
						fota_ram_clean();
						fota_retry_num = 0;

						fotadevice_checkversion();

					}
					else
					{
						Fota_printf("FOTA-->>>\r\nwrite reg file fail close fota\r\n");
						fota_retry_num = FOTA_NET_RETRY_MAXCNT;
						FotaError_Progress();
					}
				}
			}
			else
			{
				Fota_printf("FOTA-->>>\r\nadups_main_progress rg failed\r\n");
				FotaError_Progress();
			}
			break;
		case STATE_CV:
			if(info->download_status == DL_SUCCESS && info->status==FOTA_SUCCESS)
			{	
				Fota_printf("FOTA-->>>\r\nadups_main_progress cv succeed \r\n" );
				Fota_printf("FOTA-->>>\r\ninfo->deltaID=%s\r\n" , info->deltaID);

				Fota_printf("%s-01,new version:%s\r\n",__func__,info->versionName);
				//set_newversionflag(1);
				//setfotafilesize(info->fileSize);
				//report_fota_version(info->versionName,info->fileSize);
#if 0				
				if(!is_haveui()){
		
						OSATimerRef timer_id;
						/* create the OSA timer*/
						status=OSATimerCreate(&timer_id); 
						ASSERT(OS_SUCCESS == status);	
	    				if(timer_id !=0 )
    					{
    						status=OSATimerStart(timer_id, TIM3SEC, 0, fota_prepare_to_download_timer_event, NULL); /* start the timer*/
    						ASSERT(OS_SUCCESS == status);
    					}
						Fota_printf("file:%s,function:%s,line:%d\r\n", __FILE__,__func__,__LINE__);	
				}
#endif				
			
			}
			else

			{
				if(info->status == FOTA_CV_LAST_VERSION)
				{
					Fota_printf("FOTA-->>>LAST VERSION!EXIT\r\n");
					fota_retry_num = FOTA_NET_RETRY_MAXCNT;
					FotaError_Progress();
					
				}
				else
				{
					Fota_printf("FOTA-->>>\r\nadups_main_progress cv failed\r\n");
					FotaError_Progress();
				}
			}
			break;





	}


}


void FotaHttp_SetNetStatus(int network_status)
{
	rd_params->download_status= network_status;
	fota_info->download_status = network_status;

}

static fota_int32 Http_OnMessageComplete(http_parser *parser)
{
	if(parser)
	{
		return 0;
	}
    return 0;
}
int Fotadl_SaveAndCheck(fota_int32 download, fota_char *at, fota_uint32 size)
{
	fota_char read_buffer[RECV_BLOCK_SIZE];

	Fota_printf("%s-->>>download=0x%x,size=0x%x\r\n",__func__,download,size);
	if(FotaDL_DataWrite(total_download,size,(fota_uint8*)at)!=0){
		Fota_printf("FOTA-->>>\r\nError saving download file\r\n");
	    return -1;
	}		
	download += size;
	Fota_printf("FOTA-->>>\r\nsaving download file total = %d,line=%d\r\n",download,__LINE__);
#if 0	
	block_index = (fota_uint32)Adups_addr_to_block(download);
	Fota_printf("FOTA-->>>\r\nblock_index=%d,curr_block_index=%d\r\n",block_index,gcurr_block_index);
	if(block_index>gcurr_block_index)
	{
		gcurr_block_index=block_index;
		fota_set_break_point(gcurr_block_index);
	}
#endif	
	fota_memset(read_buffer,0,sizeof(read_buffer));

	if(FotaDL_DataRead((download-size),size,(fota_uint8*)read_buffer)!=0)
	{
		//adups_memfree(read_buffer);
		Fota_printf("FOTA-->>>\r\nsave body read flash fail\r\n");
		return -1;
	}		
	Fota_printf("FOTA-->>>\r\nsave body md5 update size=%d\r\n",size);

	Fota_MD5_Update(&adups_md5, (void *)read_buffer, size);
	return 0;
}


static fota_int32 Http_Savebody(http_parser *parser, const fota_char *at, fota_uint32 size)
{
    fota_int32 *pfd = (fota_int32 *)parser->data;
    //fota_uint32  Written;
	int res;
	fota_uint32 block_index=0;
	fota_int32 DownloadPercent = 0;
	//Fota_printf(  "%s-01\r\n",__func__);
	DownloadPercent = (total_download*100)/fota_info->fileSize;
	FotaReport_CurrDownloadPercent(DownloadPercent);
	Fota_printf("FOTA-->>>\r\nsave_body : size=%d,total=%d,percent = %d\r\n", size,total_download,DownloadPercent);

	Fota_printf("FOTA-->>>save_body  -------------> at = 0x%02x 0x%02x 0x%02x 0x%02x\r\n", (fota_uint8)at[0], (fota_uint8)at[1], (fota_uint8)at[2], (fota_uint8)at[3]);
	if(pfd)
	{
		;
	}

	res=Fotadl_SaveAndCheck(total_download,at,size);
	total_download += size;
    return res;
}
static fota_int32 Http_Onbody(http_parser *parser, const fota_char *at, fota_uint32 size)
{
    streambuf *res_body = parser->data;
	fota_char *ptr = res_body->data;
	Fota_printf("%s-01,at %s,size %d\r\n", __func__,at,size);	
	ptr += res_body->size;
    fota_memcpy(ptr, at, size);
    res_body->size += size;
    return 0;
}
static streambuf res;
void Fotahttp_ResponseSetting(http_parser_settings *settings,http_parser *parser)
{
		//Fota_printf( "fota_soc_recv  first time\r\n");		
    	fota_memset(settings, 0, sizeof(settings));
    	settings->on_message_complete = Http_OnMessageComplete;

		http_parser_init(parser, HTTP_RESPONSE);
		//Fota_printf(  "fota_soc_recv  fota_state = %d,line=%d\r\n", fota_state,__LINE__);		
		if(fota_state == STATE_DL)
		{
			settings->on_body = (http_data_cb)Http_Savebody;	
			outfile =555;	

			Fota_printf(  "fota_soc_recv  soc_recv---outfile = %d\r\n", outfile);	
			if(outfile<0)
			{
				FotaHttp_SetNetStatus(DL_PATH_NOT_EXIST);
				parser->data = NULL;
				Fota_printf(  "fota_soc_recv  DL_PATH_NOT_EXIST\r\n");
			}
			else
		 	{
				//Fota_printf("FOTA-->>>\r\adups_get_flash_disksize()=0x%x,fota_info->fileSize=0x%x\r\n",adups_get_flash_disksize(),fota_info->fileSize);
	    		Fota_printf("FOTA-->>>fota_soc_recv   parser.data =outfile\r\n");
				parser->data = &outfile;
				fseek(file_web,  0, SEEK_SET);
				//adups_Seek_File(outfile, 0, FOTA_SEEK_START);

		 	}
		}
		else
		{

		    res.limit = 2048;
		    res.data = fota_memalloc(res.limit);
			if(res.data != NULL)
		    fota_memset(res.data, 0, res.limit);
		    res.size = 0;
		    parser->data = &res;
		    settings->on_body = Http_Onbody;
		}
			
		fota_memset(s_fota_rx_trunk_buf, 0, FOTA_RX_TRUNK_BUF_LEN);
		s_fota_rx_trunk_buf_len = 0;
}


fota_uint32 fota_put_ring_buf(fota_uint8 *dst, fota_uint32 dstlen, fota_uint8 *src, fota_uint32 srclen, fota_uint32 dstmaxlen, fota_uint32 nodelen)
{
	fota_uint32 copylen=0,templen=0;
	fota_uint8 *pnewsrc=NULL, *pnewdst=NULL;
	
	if(!dst || !src)
		return 0;
	
	if( (srclen<=0) || (dstmaxlen<=0) || (nodelen>dstmaxlen))
		return 0;
	
	if(srclen > nodelen)
	{
		copylen = nodelen;
		pnewsrc = src+srclen-copylen;
	}
	else
	{
		copylen = srclen;
		pnewsrc = src;
	}
	
	if((dstlen+copylen) <= dstmaxlen)
	{
		pnewdst = dst+dstlen;
	}
	else
	{
		//move to left
		fota_memcpy(dst, dst+nodelen, nodelen);
		templen = dstlen - nodelen;

		pnewdst = dst+templen;
		fota_memset(pnewdst, 0, dstmaxlen-templen);		
	}
	
	fota_memcpy(pnewdst, pnewsrc, copylen);

	return copylen;
}

static void get_string(fota_char *src, fota_char *offset, fota_char **out){
	fota_char *p1=NULL;
	fota_char *p2=NULL;

	if((offset == NULL)||(src ==NULL))
		return;
		
	//Fota_printf(  "p1= %s",p1);
	p1=fota_strstr(src, offset);
	if(p1!=NULL){
		fota_int32 len=0;
		p1=p1+fota_strlen(offset);
		p2=fota_strstr(p1, "\"");
			
		//Fota_printf(  "p2= %s",p2);
		len = p2-p1;
		//Fota_printf(  "len= %d",len);
		if((len<1024) && (len>0))
		{
			*out = fota_memalloc(len+2);
			if(p1 && *out)
			{
				fota_memset(*out, 0, len+2);
				fota_strncpy(*out, p1, len);
			}
			else
				*out = NULL;
		}
		else
			*out = NULL;
		
		//Fota_printf(  "out= %s",*out);
	}
	else
		*out = NULL;
}
static void get_int(fota_char *src, fota_char *offset, fota_int32 *out){
	fota_char *p1=NULL;
	fota_char *p2=NULL;
	fota_char *p3=NULL;

	if((offset == NULL)||(src ==NULL))
		return;

	p1=fota_strstr(src, offset);
	//Fota_printf(  "p1= %s",p1);
	if(p1!=NULL){
		fota_int32 len=0;
		p1=p1+fota_strlen(offset);
		p2=fota_strstr(p1, ",");
			
		//Fota_printf(  "p2= %s",p2);
		len = p2-p1;
		//Fota_printf(  "len= %d",len);
		
		if((len<1024) && (len>0))
		{
			p3 = fota_memalloc(len+2);
			if(p1 && p3)
			{
				fota_memset(p3, 0, len+2);
				fota_strncpy(p3, p1, len);
				*out = atoi(p3);
			}
			else
				*out = 0;
		}
		else
			*out = 0;
	}
	else
		*out = 0;
	fota_memfree(p3);
}

static void fota_data_parse(http_parser *p_parser,
                            const http_parser_settings *p_settings)
{
	fota_char *buf = ((streambuf*)(p_parser->data))->data;
	fota_int32 size = ((streambuf*)(p_parser->data))->size;
	fota_int32 i=0;
	fota_char *offset;  fota_char *offset2; fota_char *p1; fota_char *p2;
	fota_int32 freesize;
	//fota_char fota_drv;
	Fota_printf(  "fota_data_parse size=%d\r\n",size);
	while(i<size){
		Fota_printf(  "buf= %s\r\n", buf+i);
		i=i+80;
	}
	
	if(fota_info->msg!=NULL)
	{
		fota_memfree(fota_info->msg);
		fota_info->msg=NULL;// clear the msg
	}
	//status
	offset="\"status\":";
	get_int(buf, offset, &(fota_info->status));
	Fota_printf(  "fota_info->status= %d", fota_info->status);

	if(fota_info->status != 1000){
		//msg
		offset="\"msg\":\"";
		get_string(buf, offset, &(fota_info->msg));
		Fota_printf(  "fota_info->msg= %s", fota_info->msg);
		return;
	}
	//content
	offset="\"content\":\"";
	if(fota_info->releaseContent!=NULL)
	{
		fota_memfree(fota_info->releaseContent);
		fota_info->releaseContent=NULL;// clear the msg
	}
	get_string(buf, offset, &(fota_info->releaseContent));
	//Fota_printf(  "fota_info->releaseContent= %s", fota_info->releaseContent);

	//publishDate
	offset="\"publishDate\":\"";
	if(fota_info->publishDate!=NULL)
	{
		fota_memfree(fota_info->publishDate);
		fota_info->publishDate=NULL;// clear the msg
	}
	get_string(buf, offset, &(fota_info->publishDate));
	//Fota_printf(  "fota_info->publishDate= %s", fota_info->publishDate);

	
	//version
	offset="\"version\":\"";
	if(fota_info->versionName!=NULL)
	{
		fota_memfree(fota_info->versionName);
		fota_info->versionName=NULL;// clear the msg
	}
	get_string(buf, offset, &(fota_info->versionName));
	//Fota_printf(  "fota_info->versionName= %s", fota_info->versionName);

	

	//battery
	offset="\"battery\",\"key_value\":\"";
	get_string(buf, offset, &p2);
	//Fota_printf(  "battery_string= %s", p2);
	if(p2)
	{
		fota_info->battery =  atoi(p2);
		fota_memfree(p2);
		p2=NULL;
	}	
	//Fota_printf(  "fota_info->battery= %d", fota_info->battery);
#if 0	
	if(adups_get_device_battery_level()*16<fota_info->battery){
		p1 = adups_strstr(buf, offset);
		offset2 = "\"key_message\":\"";
		if(p1)
		{
			if(fota_info->msg!=NULL)
			{
				adups_memfree(fota_info->msg);
				fota_info->msg=NULL;// clear the msg
			}
			get_string(p1, offset2, &(fota_info->msg));
		}
		//Fota_printf(  "fota_info->msg= %s", fota_info->msg);

	}
#endif	
	//storageSize
	offset="\"storageSize\",\"key_value\":\"";
	get_string(buf, offset, &p2);
	if(p2)
	{
		Fota_printf(  "storageSize_string= %s\r\n", p2);
		fota_info->storageSize =  atoi(p2);
		fota_memfree(p2);
		p2=NULL;
	}
	//Fota_printf(  "fota_info->storageSize= %d", fota_info->storageSize);

	freesize = Fota_GetDataPartSize();
	//not enough space for download
	if(fota_info->storageSize > freesize && fota_info->msg== NULL){
		p1 = fota_strstr(buf, offset);
		offset2 = "\"key_message\":\"";
		if(p1)
			get_string(p1, offset2, &(fota_info->msg));
		//Fota_printf(  "fota_info->msg= %s", fota_info->msg);
	}
	
	//Version info:

	//md5sum
	offset="\"md5sum\":\"";
	if(fota_info->md5sum!=NULL)
	{
		fota_memfree(fota_info->md5sum);
		fota_info->md5sum=NULL;// clear the msg
	}
	get_string(buf, offset, &(fota_info->md5sum));
	//Fota_printf(  "adups :fota_info->md5sum= %s", fota_info->md5sum);

	//fileSize
	offset="\"fileSize\":";
	get_int(buf, offset, &(fota_info->fileSize));
	//Fota_printf(  "fota_info->fileSize= %d", fota_info->fileSize);
	if(fota_info->fileSize > freesize && fota_info->msg== NULL){
		fota_info->msg = fota_memalloc(fota_strlen("no enough space") + 2);
		fota_memcpy(fota_info->msg, "no enough space", fota_strlen("no enough space"));
		//fota_info->msg = "no enough space";
	}
		
	//deltaID
	offset="\"deltaID\":\"";
	if(fota_info->deltaID!=NULL)
	{
		fota_memfree(fota_info->deltaID);
		fota_info->deltaID=NULL;// clear the msg
	}
	get_string(buf, offset, &(fota_info->deltaID));
	//Fota_printf(  "fota_info->deltaID= %s", fota_info->deltaID);
	
	//deltaUrl
	offset="\"deltaUrl\":\"";
	if(fota_info->deltaUrl!=NULL)
	{
		fota_memfree(fota_info->deltaUrl);
		fota_info->deltaUrl=NULL;// clear the msg
	}
	get_string(buf, offset, &(fota_info->deltaUrl));
	//Fota_printf(  "fota_info->deltaUrl= %s", fota_info->deltaUrl);

	//deviceSecret
	offset="\"deviceSecret\":\"";
	if(fota_info->deviceSecret!=NULL)
	{
		fota_memfree(fota_info->deviceSecret);
		fota_info->deviceSecret=NULL;// clear the msg
	}
	get_string(buf, offset, &(fota_info->deviceSecret));
	Fota_printf(  "fota_info->deviceSecret= %s", fota_info->deviceSecret);


	//deviceId
	offset="\"deviceId\":\"";
	if(fota_info->deviceId!=NULL)
	{
		fota_memfree(fota_info->deviceId);
		fota_info->deviceId=NULL;// clear the msg
	}
	get_string(buf, offset, &(fota_info->deviceId));
	Fota_printf(  "fota_info->deviceId= %s", fota_info->deviceId);
}



int Fotahttp_RecvPerform (http_parser_settings *p_settings,
							http_parser *p_parser,
                            const char *buf,
                            size_t len)
{
	enum http_errno err;
	fota_uint32 trunk_len = 0;
	//trunk buffer
	//Fota_printf(  "%s-01,buf=%x,len=%d\r\n",__func__,buf,len);
	if(fota_state != STATE_DL)
	{
		trunk_len = 0;
		Fota_printf(  "%s-011,s_fota_rx_trunk_buf_len=%d,len=%d\r\n",__func__,s_fota_rx_trunk_buf_len,len);
		trunk_len = fota_put_ring_buf(s_fota_rx_trunk_buf, s_fota_rx_trunk_buf_len, (fota_uint8 *)buf, len, FOTA_RX_TRUNK_BUF_LEN, FOTA_RX_TRUNK_NODE_LEN);
		s_fota_rx_trunk_buf_len += trunk_len;
		Fota_printf(  "%s-012,trunk_len=%d,s_fota_rx_trunk_buf_len=%d\r\n",__func__,trunk_len,s_fota_rx_trunk_buf_len);
		Fota_printf("%s\r\n",s_fota_rx_trunk_buf);

	}

	http_parser_execute(p_parser, p_settings, buf, len);
	err = (enum http_errno)p_parser->http_errno;
	Fota_printf("FOTA-->>>\r\nHTTP parsing error: %s - %s\r\n", http_errno_name(err), http_errno_description(err));		
	if (err != HPE_OK)
	{
		FotaHttp_SetNetStatus(DL_NETWORK_ERROR);
		Fota_printf("FOTA-->>>\r\nfota_soc_recv  STATE_DL DL_NETWORK_ERROR\r\n");	
		return -1;
	} 
	// complete flag
	if(fota_state == STATE_DL)
	{
		//Fota_printf(  "%s-02\r\n",__func__);
		if((fota_int32)total_download>= fota_info->fileSize)
		{
			FotaHttp_SetNetStatus(DL_SUCCESS);
			Fota_printf("FOTA-->>>\r\nfota_soc_recv file download complete!\r\n");
			PlatTaskSleep(200);				
			return 0;
		}
	}
	else if(fota_strstr((fota_char *)s_fota_rx_trunk_buf, "0\r\n\r\n")!=NULL)
	{
		Fota_printf("FOTA-->>>\r\nfota_soc_recv 0rnrn   End! line=%d\r\n",__LINE__);
		FotaHttp_SetNetStatus(DL_SUCCESS);
		fota_data_parse(p_parser,p_settings);
		return 0;
	}	
	//Fota_printf(  "%s-03\r\n",__func__);
	return 1;
}



void Fotahttp_ResponsePost(http_parser_settings *p_settings,http_parser *p_parser)
{

	// need to report the  result whatever download success or failed!
	if(fota_state == STATE_DL)
	{
		rd_params->download_end = fota_gettime();
		//Fota_printf("FOTA-->>>download_end %d \r\n",rd_params->download_end);
		rd_params->download_size = total_download;
		
		Fota_printf("FOTA-->>>\r\ntotal lendth is %d fota_info->fileSize length is %d\r\n",total_download,fota_info->fileSize);
		
		if((fota_int32)total_download< fota_info->fileSize)
		{
			FotaHttp_SetNetStatus(DL_NETWORK_ERROR);
			Fota_printf("FOTA-->>>\r\nfota_soc_recv file download length is not enough! \r\n");
		}
	}
	else
	{
		//Fota_printf("FOTA-->>>fota_soc_recv  adups_memfree line=%d,fota_state=%d\r\n",__LINE__,fota_state); 
		if(p_parser && (p_parser->data) && ((streambuf*)(p_parser->data))->data)
		{
			Fota_printf("FOTA-->>>fota_soc_recv  fota_memfree line=%d\r\n",__LINE__); 
		    fota_memfree(((streambuf*)(p_parser->data))->data);
		}
	}

	

}






#define TRYTIME (5)
/**
 * @brief    Send Fota device Register to the server.
 *
 * @param    None
 *
 * @return   None
 *
 * @details  This function is used to send fota device register to the server. 
 *           and must be called before start to get new deviceversion.
 */
void fotadevice_register(void)
{
	int socketfd;
	fota_state = STATE_RG;
	FotaReport_CurrState(fota_state);
	FotaParams_init();
	socketfd= fota_connect_to_server((UINT8*)fota_tcp_server_ip,fota_tcp_server_port);
	Fota_printf("FOTA-->>>adups_register: creat socket socketfd=%d\r\n",socketfd);

	if(socketfd < 0)
	{
		Fota_printf("FOTA-->>>adups_register: creat socket failed\r\n");
		FotaError_Progress();
		return;
	}
	fotahttp_requestprocess(socketfd,fota_state,TRYTIME);

}

#define CHECK_VERSION_URL           "/ota/open/checkVersion"
#define REGISTER_URL   				"/register"
#if 0
void dmfota_register(void)
{
	int ret;
	char * posturl=NULL;
	httpdatabuf body;
	httpdatabuf * postbuffer=&body;

	INT32 limit=0;
	INT32 size=0;
	Fota_printf("+++++++++++++%s+++++++\r\n",__func__);

	fota_state = STATE_RG;
	FotaReport_CurrState(fota_state);
	limit=1024;
	posturl=fota_memalloc(1024);
	if(posturl!=NULL)
	{
		size = fota_snprintf(posturl, limit,
					"http://%s/%s/%s",
					fota_tcp_server_ip, REGISTER_URL,cv_params->productid);
		posturl[size]=0;
	}
	else{
		Fota_printf("FOTA-->>>\r\nadups RD malloc fail!!!\r\n");
		FotaError_Progress();
		return;

	}

	postbuffer->data = fota_memalloc(512);
	postbuffer->limit = 512;
	if((postbuffer->data!=NULL) && (rd_params!=NULL))
	{		
		fota_make_json_request(fota_state,postbuffer->data,512);
		postbuffer->size =fota_strlen(postbuffer->data);

	}
	else{
		Fota_printf("FOTA-->>>\r\nadups RD malloc fail!!!\r\n");
		FotaError_Progress();
		return;

	}
	ret=fotahttp_requestpost(posturl,postbuffer);
	
	if(posturl!=NULL)
		fota_memfree(posturl);
	if(postbuffer->data!=NULL)
		fota_memfree(postbuffer->data);
		
	if(ret < 0)
	{
		Fota_printf("FOTA-->>>adups_register: creat socket failed\r\n");
		FotaError_Progress();
		return;
	}
	//while(1);
	
}
#endif

void fotadevice_checkversion(void)
{
	int socketfd; 
	
	fota_state = STATE_CV;
	FotaReport_CurrState(fota_state);
	socketfd = fota_connect_to_server((UINT8*)fota_tcp_server_ip,fota_tcp_server_port);
	fota_state = STATE_CV;
	FotaReport_CurrState(fota_state);
	if(socketfd < 0)
	{
		Fota_printf("FOTA-->>>%s: creat socket failed\r\n",__func__);
		FotaError_Progress();
		//set_newversionflag(ENSOCKET);
		return;
	}
	
	fotahttp_requestprocess(socketfd,fota_state,TRYTIME);
}


void fotadevice_download(void)
{
	int socketfd = fota_connect_to_server((UINT8*)fota_tcp_server_ip,fota_tcp_server_port);
	fota_state = STATE_DL;
	total_download=0;
	FotaReport_CurrState(fota_state);
	if(socketfd < 0)
	{
		Fota_printf("FOTA-->>>%s: adups_connect_to_server failed\r\n",__func__);
		FotaError_Progress();
		return;
	}
	fotahttp_requestprocess(socketfd,fota_state,TRYTIME);
}

void fotadevice_download_prepare(void)
{
	fota_char *url ;
	struct http_parser_url url_info;
	fota_char *p1;fota_char *p2;fota_char *p3;
	Fota_printf("FOTA-->>>\r\nadups_fota_download ----------start \r\n");
	fota_state = STATE_DL;
	FotaReport_CurrState(fota_state);
	fota_memset(rd_params,0,sizeof(DownParamInfo));
	rd_params ->mid = cv_params->mid;
	rd_params->download_status= DL_SUCCESS;
	rd_params->productid= fota_get_device_product_id();
	rd_params->productsec= fota_get_device_product_sec();
	fota_info->download_status= DL_SUCCESS;
	rd_params->download_start = fota_gettime();
	Fota_printf("FOTA-->>>download_start time = %d \r\n", rd_params->download_start);
    rd_params->deltaID = fota_info->deltaID;
    
    url = fota_info->deltaUrl;
	//url=downloadurl;
    http_parser_parse_url(url, fota_strlen(url), 0, &url_info);
	Fota_printf("FOTA-->>>url=%s\r\n", url);
	fotadownload_result = 0;
#ifdef ADUPS_REPORT_DL
	adups_set_report_str((fota_uint8 *)url);
	Fota_printf("FOTA-->>>\r\nadups_report_dl is %s\r\n",adups_report_dl);
#endif

	 file_web = fopen(file_nameforsave, "wb+");
	
	p1=fota_strstr(url, "http://");
	Fota_printf("FOTA-->>>p1= %s\r\n",p1);
	if(p1!=NULL){
		fota_int32 len;
		p1=p1+fota_strlen("http://");
		p2=fota_strstr(p1, "/");
			
		Fota_printf("FOTA-->>>p2= %s\r\n",p2);
		len = p2-p1;
		Fota_printf("FOTA-->>>len= %d\r\n",len);

		if(FOTA_DOWNLOAD_DOMIAN!=NULL)
		{
			fota_memfree(FOTA_DOWNLOAD_DOMIAN);
			FOTA_DOWNLOAD_DOMIAN=NULL;
		}
		
		FOTA_DOWNLOAD_DOMIAN = fota_memalloc(len+2);
		fota_memset(FOTA_DOWNLOAD_DOMIAN, 0, len+2);
		fota_strncpy(FOTA_DOWNLOAD_DOMIAN, p1, len);
		p3 = fota_strstr(p2, ":");
		if(p3!=NULL)
		{
			len = p3-p2;
		}
		len = fota_strlen(p2);
		if(DOWNLOAD_DELTA_URL!=NULL)
		{
			fota_memfree(DOWNLOAD_DELTA_URL);
			DOWNLOAD_DELTA_URL=NULL;
		}
		DOWNLOAD_DELTA_URL = fota_memalloc(len+2);
		fota_memset(DOWNLOAD_DELTA_URL, 0, len+2);
		fota_strncpy(DOWNLOAD_DELTA_URL, p2, len);
	}
	
	Fota_printf("FOTA-->>>\r\nadups_fota_download   domain: %s  port:%d\r\n",FOTA_DOWNLOAD_DOMIAN, FOTA_DOWNLOAD_PORT);
	Fota_printf("FOTA-->>>\r\nadups_fota_download 2  url_path: %s\r\n", DOWNLOAD_DELTA_URL);
	fotadevice_download();
	fclose(file_web);

}
















void adups_start_send_update_result(void)
{
	int socketfd;
	fota_state = STATE_RU;
	FotaReport_CurrState(fota_state);
	socketfd= fota_connect_to_server((UINT8*)fota_tcp_server_ip,fota_tcp_server_port);

	if(socketfd < 0)
	{
		Fota_printf("FOTA-->>>adups_start_send_update_result: creat socket failed\r\n");
		FotaError_Progress();
		return;
	}
	fotahttp_requestprocess(socketfd,fota_state,TRYTIME);
}


void fota_start_send_update_result_pre(void)
{
	
	fota_uint32 result;
	if(fota_runing == FOTA_TRUE || fota_get_reg_info() == FOTA_FALSE)
	{
		Fota_printf("FOTA-->>>adups_start_send_update_result: fota is running\r\n");
		return;
	}
	fota_runing = FOTA_TRUE;
	//adups_fota_init();
	result = FotaGetUpdateResult();
	if(result  == 2 || ru_params->deltaID == NULL) //no update event
	{
		fota_runing = FOTA_FALSE;
		return;
	}
	else if(result  == 0)
	{
		ru_params->update_status = 1; //succeed
	}
	else
	{
		ru_params->update_status = 99;
	}
	adups_start_send_update_result();


}






void fota_get_new_version(void)
{
	if(fota_runing == FOTA_TRUE)
	{
		Fota_printf("FOTA-->>>adups: fota is running\r\n");
		return;
	}

	Fota_printf("FOTA-->>>%s\r\n",__func__); 
	fota_runing = FOTA_TRUE;
	
	FotaParams_init();

	if(fota_get_reg_info() == FOTA_FALSE)
	{
		//need reg first
		fotadevice_register();
		//dmfota_register();
		return;
	}
	//prepare check version
	fotadevice_checkversion();

	
}






























