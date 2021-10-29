


#include <stdio.h>
#include "fota_typedef.h"

#ifdef MINGW64
#include <winsock2.h>
#define Fota_printf printf
#else
#include "asr_crane.h"
#include "ip.h"
#include <sys/socket.h>
#define Fota_printf uart_printf
#endif


#include "fota_md5.h"
#include "fota_http_parser.h"
#include "fota_stdlib.h"
#include "fota_config.h"

#include "fota_cjson.h"
#include "fota_hmd5.h"
#include "fota_define.h"

#define CHECK_VERSION_URL           "/ota/open/checkVersion"
#define REGISTER_URL   				"/register"

extern MobileParamInfo * fota_get_cv_params(void);
extern VersionInfo * fota_get_fota_info(void);
extern UpgradeParamInfo * fota_get_ru_params(void);
extern DownParamInfo * fota_get_rd_params(void);
extern void fotahttp_soc_close(int sockfd);
extern fota_char* fota_get_fota_pid(void);
extern fota_char* fota_get_fota_psec(void);
extern FotaState fota_get_fota_state(void);
extern UINT8* fota_get_tcp_server_ip(void);
extern fota_char* fota_get_report_dl(void);
extern FotaState fota_state;

extern UINT8 * fota_tcp_server_ip;

extern fota_char *DOWNLOAD_DELTA_URL;
extern fota_char *FOTA_DOWNLOAD_DOMIAN;
extern UINT16 ADUPS_DOWNLOAD_PORT;
extern fota_uint32 total_download;

void free_request(Request *req)
{
    if (req && req->body.data) {
       //Fota_printf("ADUPS-->>>free_request   req->body.data \r\n"); 
        fota_memfree(req->body.data);
    }

    if (req && req->header.data) {
       //Fota_printf("ADUPS-->>>free_request   req->header.data \r\n"); 
        fota_memfree(req->header.data);
    }

    //Fota_printf("ADUPS-->>>free_request   req \r\n"); 
	if(req)
	{
    	fota_memfree(req);
		req = NULL;
	}
}

void fota_make_json_request(fota_int8 state,fota_char *ptr,fota_uint32 len)
{
	cJSON *root=NULL;
	static fota_char *out_string=NULL;
	fota_char intstr[5];
	fota_uint32 adups_time=0;
	MobileParamInfo *cv_params = fota_get_cv_params();
	UpgradeParamInfo *ru_params = fota_get_ru_params();
	DownParamInfo *rd_params = fota_get_rd_params();
	VersionInfo *fota_info =  fota_get_fota_info();
	fota_char* fota_pid = fota_get_fota_pid();
	fota_char* fota_psec = fota_get_fota_psec();

	fota_char* adups_report_dl = fota_get_report_dl();
	fota_char *signdata="96e6fb11c2d929ea0e9ef38a10c0b00a";
	printf("%s-01\r\n",__func__);
	switch(state)
	{
		case STATE_RG:
			root = fota_cJSON_CreateObject();
    			cJSON_AddStringToObject(root, "mid", cv_params->mid);
    			cJSON_AddStringToObject(root, "oem", cv_params->oem);
    			cJSON_AddStringToObject(root, "models", cv_params->models);
    			cJSON_AddStringToObject(root, "platform", cv_params->platform);
    			cJSON_AddStringToObject(root, "deviceType", cv_params->device_type);

			//adups_time=adups_get_utc_time_sec(&tmp_time);
				adups_time = fota_gettime();
				signdata=fota_hmd5_mid_pid_psecret(cv_params->mid,cv_params->productid,cv_params->productsec,adups_time);

   				cJSON_AddNumberToObject(root, "timestamp",adups_time);	
  			
    			cJSON_AddStringToObject(root, "sign", signdata);

    			cJSON_AddStringToObject(root, "sdkversion", FOTA_SDK_VER);
 
    			cJSON_AddStringToObject(root, "appversion", FOTA_APP_VER);

    			cJSON_AddStringToObject(root, "version",cv_params->version);

    			cJSON_AddStringToObject(root, "networkType",DEVICE_NETWORKTYPE);
			out_string = fota_cJSON_Print(root); 
			Fota_printf("ADUPS-->>>\r\nRG:%s\r\n", out_string);	
			break;
		case STATE_CV:
			root = fota_cJSON_CreateObject();
    			cJSON_AddStringToObject(root, "mid", cv_params->mid);
    			cJSON_AddStringToObject(root, "version",cv_params->version);
				adups_time = fota_gettime();
   				cJSON_AddNumberToObject(root, "timestamp",adups_time);	
    			cJSON_AddStringToObject(root, "sign", fota_hmd5_did_pid_psecret(fota_pid,cv_params->productid,fota_psec,adups_time));
		
				out_string = fota_cJSON_Print(root); 
				Fota_printf("ADUPS-->>>\r\nCV:%s\r\n", out_string);	
			break;
		case STATE_RD:
			root = fota_cJSON_CreateObject();
    			cJSON_AddStringToObject(root, "mid", rd_params->mid);
    			cJSON_AddStringToObject(root, "deltaID",rd_params->deltaID);
				
			fota_memset(intstr,0,5);
			fota_snprintf(intstr, 2, "%d", rd_params->download_status);
			fota_info->download_status = rd_params->download_status;
    			cJSON_AddStringToObject(root, "downloadStatus",intstr);

			adups_time = rd_params->download_start;
			fota_info->download_start=adups_time;
   			cJSON_AddNumberToObject(root, "downStart",adups_time);

			adups_time = rd_params->download_end;
			fota_info->download_end=adups_time;
   			cJSON_AddNumberToObject(root, "downEnd",adups_time);

   			cJSON_AddNumberToObject(root, "downSize",rd_params->download_size);
			if(fota_strlen(rd_params->download_ip)!=0)
   				cJSON_AddStringToObject(root, "downIp",rd_params->download_ip);			

			//adups_time=adups_get_utc_time_sec(&tmp_time);
			adups_time = fota_gettime();
   			cJSON_AddNumberToObject(root, "timestamp",adups_time);
			
    			cJSON_AddStringToObject(root, "sign", fota_hmd5_did_pid_psecret(fota_pid,rd_params->productid,fota_psec,adups_time));

			if(fota_strlen(adups_report_dl)!=0&&fota_info->download_status!=1)
   				cJSON_AddStringToObject(root, "extStr",adups_report_dl);				
			out_string = fota_cJSON_Print(root); 
			Fota_printf("ADUPS-->>>\r\nRD:%s\r\n", out_string);
			break;
		case STATE_RU:
			root = fota_cJSON_CreateObject();
    			cJSON_AddStringToObject(root, "mid", ru_params->mid);
    			cJSON_AddStringToObject(root, "deltaID",ru_params->deltaID);

			fota_memset(intstr,0,5);
			fota_sprintf(intstr,"%d", (fota_uint8)ru_params->update_status);
			Fota_printf(  " RU status=%d,%d,%d,%d\r\n", ru_params->update_status,intstr[0],intstr[1],intstr[2]);
   			cJSON_AddStringToObject(root, "updateStatus",intstr);
			fota_info->update_status = ru_params->update_status;

			//adups_time=adups_get_utc_time_sec(&tmp_time);
			adups_time = fota_gettime();
   			cJSON_AddNumberToObject(root, "timestamp",adups_time);
			
    			cJSON_AddStringToObject(root, "sign", fota_hmd5_did_pid_psecret(fota_pid,ru_params->productid,fota_psec,adups_time));
			if(fota_strlen(ru_params->extendString)!=0&&ru_params->update_status!=1)
   				cJSON_AddStringToObject(root, "extStr",ru_params->extendString);
			
			out_string = fota_cJSON_Print(root); 
			Fota_printf("ADUPS-->>>\r\nRU:%s\r\n", out_string);			
			break;
		default:
			break;
		
	}

	if(root!=NULL)
		fota_cJSON_Delete(root);
	
	if(out_string!=NULL)
	{
		if(fota_strlen(out_string)>len)
			fota_strncpy(ptr,out_string,len);
		else
			fota_strcpy(ptr,out_string);
		fota_cJSON_free(out_string);
	}
	


}

Request *make_request(fota_socint soc_id)
{
    Request *req=NULL;
    streambuf *header=NULL;
    streambuf *body=NULL;
	fota_char *wptr=NULL;
	//adups_char *reqtxt=NULL;
	fota_int32 limit=0;
	fota_int32 size=0;
	MobileParamInfo *cv_params = fota_get_cv_params();
	UpgradeParamInfo *ru_params = fota_get_ru_params();
	DownParamInfo *rd_params = fota_get_rd_params();
	VersionInfo *fota_info =  fota_get_fota_info();
	FotaState fota_state = fota_get_fota_state();
	fota_char* fota_pid = fota_get_fota_pid();
	UINT8* fota_tcp_server_ip = fota_get_tcp_server_ip();

	Fota_printf("ADUPS-->>>\r\n make_request ------------- start\r\n");
	Fota_printf("ADUPS-->>>\r\n make_request size %d\r\n",sizeof(Request));
	req = fota_memalloc(sizeof(Request));
	Fota_printf("ADUPS-->>>\r\n make_request  malloc finshed\r\n");
	if(req == NULL)
		return NULL;
	Fota_printf("ADUPS-->>>\r\n make_request  malloc succeed\r\n");	
	fota_memset(req, 0, sizeof(Request));
    header = &req->header;
    body = &req->body;
	header->size = 0;
	body->size = 0;
	Fota_printf("ADUPS-->>>\r\n make_request  malloc 1024\r\n");
    header->data = fota_memalloc(1024);
    Fota_printf("ADUPS-->>>\r\n make_request  malloc 1024 finshed\r\n");
	if(header->data == NULL)
		return NULL;
	Fota_printf("ADUPS-->>>\r\n make_request  malloc succeed\r\n");

    header->limit = 1024;
    wptr = header->data;
    limit = header->limit;
	fota_memset(wptr, 0, 1024);
	Fota_printf("ADUPS-->>> make_request   ---   fota_state=%d\r\n", fota_state);
	if(fota_state == STATE_CV)
	{
		size = fota_snprintf(wptr, limit,
	                    "%s /product/%s/%s/ota/checkVersion.php HTTP/1.1\r\nAccept: */*\r\nHost: %s\r\n\r\n",
	                    "POST", cv_params->productid,fota_pid, fota_tcp_server_ip);
	    header->size += size;

		body->data = fota_memalloc(512);
		body->limit = 512;
		if((body->data!=NULL) && (cv_params!=NULL))
		{
			fota_make_json_request(fota_state,body->data,512);
			body->size +=fota_strlen(body->data);

		}
		else
			Fota_printf("ADUPS-->>>\r\nadups CV malloc fail!!!\r\n");
	}
	else if(fota_state == STATE_RG){
		printf("%s-101\r\n",__func__);
#if 1		
		size = fota_snprintf(wptr, limit,
                        "%s %s/%s.php HTTP/1.1\r\nAccept: */*\r\nHost: %s\r\n\r\n",
                        "POST", REGISTER_URL,cv_params->productid, fota_tcp_server_ip);
#else						
		size = fota_snprintf(wptr, limit,
                        "%s %s HTTP/1.1\r\nAccept: */*\r\nHost: %s\r\n\r\n",
                        "POST", REGISTER_URL, fota_tcp_server_ip);	
#endif											
	    header->size += size;
		printf("%s-102\r\n",__func__);
		body->data = fota_memalloc(512);
	    body->limit = 512;
		printf("%s-103\r\n",__func__);
		fota_make_json_request(fota_state,body->data,512);
		printf("%s-104\r\n",__func__);
		body->size +=fota_strlen(body->data);
	}

	else if(fota_state == STATE_DL)
	{
		if(fota_strlen(fota_pid)==0)
		{
#if 0			
			size = fota_snprintf(wptr, limit,
			       "%s %s HTTP/1.1\r\nAccept: */*\r\nHost: %s\r\n\r\n",
			       "GET", DOWNLOAD_DELTA_URL, FOTA_DOWNLOAD_DOMIAN);
#else				   
			size = fota_snprintf(wptr, limit,
			       "%s %s HTTP/1.1\r\nAccept: */*\r\nHost: %s\r\nRange: bytes=%d-%d\r\n\r\n",
			       "GET", DOWNLOAD_DELTA_URL, FOTA_DOWNLOAD_DOMIAN,total_download,(fota_info->fileSize-1));	  
#endif				    
		}
		else
		{
#if 0			
			size = fota_snprintf(wptr, limit,
			       "%s %s?%s HTTP/1.1\r\nAccept: */*\r\nHost: %s\r\n\r\n",
			       "GET", DOWNLOAD_DELTA_URL, fota_pid,FOTA_DOWNLOAD_DOMIAN);
#else
			size = fota_snprintf(wptr, limit,
			       "%s %s?%s HTTP/1.1\r\nAccept: */*\r\nHost: %s\r\nRange: bytes=%d-%d\r\n\r\n",
			       "GET", DOWNLOAD_DELTA_URL, fota_pid,FOTA_DOWNLOAD_DOMIAN,total_download,(fota_info->fileSize-1));
#endif				   
		}
		header->size += size;
	}
	
	else if(fota_state == STATE_RD)
	{
		size = fota_snprintf(wptr, limit,
                        "%s /product/%s/%s/ota/reportDownResult HTTP/1.1\r\nAccept: */*\r\nHost: %s\r\n\r\n",
                        "POST", rd_params->productid,fota_pid, fota_tcp_server_ip);
		
		Fota_printf(  "make_request ADUPS_FOTA_DOMIAN is %s\r\n",fota_tcp_server_ip);	
	    header->size += size;

		body->data = fota_memalloc(1024);
	    body->limit = 1024;
		if((body->data!=NULL) && (rd_params!=NULL))
		{		

		fota_make_json_request(fota_state,body->data,1024);
		body->size +=fota_strlen(body->data);
		}
		else
			Fota_printf("ADUPS-->>>\r\nadups RD malloc fail!!!\r\n");
	}
	
	else if(fota_state == STATE_RU)
	{
		size = fota_snprintf(wptr, limit,
		                "%s /product/%s/%s/ota/reportUpgradeResult HTTP/1.1\r\nAccept: */*\r\nHost: %s\r\n\r\n",
		                "POST", ru_params->productid, fota_pid,fota_tcp_server_ip);
	    header->size += size;

		body->data = fota_memalloc(512);
	    body->limit = 512;
		if((body->data!=NULL) && (ru_params!=NULL))
		{
		fota_make_json_request(fota_state,body->data,512);
		body->size +=fota_strlen(body->data);	  	
		}
		else
	Fota_printf("ADUPS-->>>\r\nadups RU malloc fail!!!\r\n");
	}

	else
	{
		free_request(req);
		fotahttp_soc_close(soc_id);
	}
	printf("%s-01\r\n",__func__);
    if (body->size) 
	{
        fota_char *wptr = header->data + header->size - 2;
		 fota_uint32 n;
		//Fota_printf("ADUPS-->>>\r\n make_request ------------header->size =%d  header->limit =%d \r\n",header->size, header->limit);
        limit = header->limit - header->size + 2;
		//Fota_printf("ADUPS-->>>\r\n make_request ------------limit =%d\r\n",limit);
        n = fota_snprintf(wptr, limit, "Content-Type: application/json\r\nContent-Length: %d\r\n\r\n", body->size);
		//Fota_printf("ADUPS-->>> make_request wptr is %s \r\n",wptr);

		header->size += (n - 2);
		fota_strcat(header->data, body->data);
    }

	printf("%s-02\r\n",__func__);

    return req;
}




