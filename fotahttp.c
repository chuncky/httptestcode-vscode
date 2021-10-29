/**************************************************************************//**
 * @file     fotahttp.c
 * @version  V1.00
 * @brief    Fota Http Client Source File
 *
 * @note
 * Copyright (C) 2021 ASR Microelectronics, Ltd. Co. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

#ifdef MINGW64
#include <winsock2.h>
#define Fota_printf printf
#else
#include <sys/socket.h>
#define Fota_printf uart_printf
#endif

#include "fota_typedef.h"
#include "fota_http_parser.h"
#include "fota_http.h"
#include "fota_stdlib.h"
#include "platformbase.h"

static http_parser_settings settings;
static http_parser parser;

static http_parser_settings *s_settings = &settings;
http_parser *s_parser = &parser;
extern VersionInfo *fota_info;
extern FotaState fota_state;

/**
 * @brief    Fota state report to UI app.
 *
 * @param	 None
 * 
 * @return   None
 *
 * @details  This function is used to report the fota state the UI. 
 */       
int PdnIsconnected(void)
{
	int connectcnt;
	connectcnt=0;
#if !defined(MINGW64)
    /* Wait for the default pdn is connected. */
    Fota_printf("FOTA-->>> adups_connect_to_server:waitting pdn active!\r\n");

	while(!get_linkstas(getDefaultCidNum() - 1) && connectcnt < 20)
	{
		PlatTaskSleep(200);
		++connectcnt;
	}
	Fota_printf("FOTA-->>>%s,line=%d  is_pdp_connected(1)=%d\r\n",__FUNCTION__,__LINE__,is_pdp_connected(1));
	Fota_printf("FOTA-->>>%s,line=%d  is_pdp_connected(0)=%d\r\n",__FUNCTION__,__LINE__,is_pdp_connected(0));
	if (connectcnt>=20){
		Fota_printf("FOTA-->>> adups_connect_to_server:pdp connect error!\r\n");
		return -1;
	}
#endif
	return 0;

}

void fotahttp_soc_close(int sockfd)
{
    socketclose(sockfd);

#ifdef FIELD_TEST
    DS_Unbind_EpsChId_from_NetIf(adups_pdphandle);
#endif
}

/**
 * @brief    Fota state report to UI app.
 *
 * @param[in]  host  current fota state
 * 
 * @param[in]  port  current fota state
 *
 * @return   None
 *
 * @details  This function is used to report the fota state the UI. 
 *           
 */
int fota_connect_to_server(UINT8* host,UINT16 port)
{
    //struct _sockaddr_in_ addr;
    struct sockaddr_in addr;
    int sockfd = 0;
    int ret;
    int i = 0;
	int nNetTimeout=20*1000;

	struct hostent *phostent = NULL;
	//UINT8 ipp[]={103,40,232,167};

	Fota_printf("FOTA-->>> fotahttp_connect_to_server:host:%s\r\n",host);
	phostent = gethostbyname((char *)host);
	if(phostent == NULL)
	{
		return -1;
	}
    Fota_printf("FOTA-->>> hostname: %s\r\n",phostent->h_name);
    Fota_printf("FOTA-->>> ip: %d.%d.%d.%d\r\n",(unsigned char)phostent->h_addr_list[i][0],(unsigned char)phostent->h_addr_list[i][1],(unsigned char)phostent->h_addr_list[i][2],(unsigned char)phostent->h_addr_list[i][3]);

    memcpy(&(addr.sin_addr.s_addr),&phostent->h_addr_list[i][0], 4);
    //memcpy(&(addr.sin_addr.s_addr),&ipp[0], 4);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    
    sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        Fota_printf("FOTA-->>> connect_to_server socket() failed: %d\r\n", sockfd);
        return -1;
    }
	Fota_printf("FOTA-->>> socket: sockfd=%d\r\n",sockfd);

    /* Connect to the server */
    ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr));
    Fota_printf("FOTA-->>> connect: ret=%d\r\n",ret);
    if (ret < 0)
    {
	    socketclose(sockfd);
		return -1;
    }

	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&nNetTimeout,sizeof(nNetTimeout));
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (void *)&nNetTimeout,sizeof(nNetTimeout));
    
    Fota_printf("FOTA-->>> connect to server OK\r\n");  
    return  sockfd;
}


void dump_data(fota_uint8 *data, fota_uint32 len)
{
	fota_uint32 i =0;
	Fota_printf("\r\n+++++++++++++%s++++++++\r\n",__func__);
	while(i < len)
	{
		//Fota_printf("FOTA-->>>0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \r\n",
		//data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6],data[i+7],data[i+8],data[i+9],data[i+10],data[i+11],data[i+12],data[i+13],data[i+14],data[i+15]);
		Fota_printf("%c",data[i]);
		i++;
	}
	Fota_printf("\r\n------%s-------\r\n",__func__);	
}



fota_int32  fotahttp_send(fota_socint soc_id,Request* req )
{
	fota_int32 result;
	//fota_int32 written_len = 0;
	fota_int32 len = strlen(req->header.data);
	//Fota_printf("FOTA-->>>\r\nadups_soc_send  soc_id = %d,g_tcp_socket_id:%d\r\n", soc_id,adups_socket_mgr.fota_tcp_socket_id);
	//Fota_printf("\r\n%s http request:>> header\r\n %s\r\n",__func__,req->header.data);
	//Fota_printf("\r\n%s http request:>> body\r\n %s\r\n",__func__,req->body.data);

   	result =send(soc_id, req->header.data, len, 0);
	
	//Fota_printf("FOTA-->>>\r\nadups_soc_send num of byte sent=%d\r\n", result);

	if (result > 0)
	{
		//Fota_printf("FOTA-->>>\r\nadups_soc_send   SOC_SUCCESS \r\n");
		return E_FOTA_SUCCESS;
	}
	else
	{
		 Fota_printf("FOTA-->>>\r\nadups_soc_send	 SOC_ERROR \r\n"); 
		 return E_FOTA_FAILURE;		 
	 }
}


fota_int32 fotahttp_recv(fota_socint soc_id,fota_char* buf,fota_int32 len,fota_uint32 flag)
{
	fota_int32 result = -1;
	result = recv(soc_id, buf, len, flag);
	if (result > 0)
	{
		Fota_printf("Fota-->>>\r\n fotahttp_recv succeed result=%d,  line = %d\r\n", result,__LINE__);
		if(fota_state!= STATE_DL ){
			dump_data((fota_uint8 *)buf,result);
		}
		return 	result;	
	}
	else
	{
		return E_FOTA_FAILURE;	
	}
}



void fotahttp_getresponse(int soc_id,fota_int32 trymaxnum)
{
	static fota_char buf[RECV_BLOCK_SIZE];
	fota_int32 result = -1;
	static fota_int32 HttpRecvNullTime = 0;	
	fota_uint32 trunk_len = 0;
	int res;

	//fota_uint32 file_len=0;
	Fota_printf( "%s  line=%d\r\n",__func__,__LINE__); 

	Fotahttp_ResponseSetting(s_settings,s_parser);

	HttpRecvNullTime = 0;//adups_task
	//Fota_printf("+++++++++++++++fota_soc_recv  line=%d++++++++++++++++++++++\r\n",__LINE__); 

    for (;;) 
	{
		fota_memset(buf, 0, sizeof(buf));
		if(s_parser->data == NULL)
		{
			Fota_printf("FOTA-->>>\r\nfota_soc_recv  s_parser->data ==NULL\r\n");	
			break;
		}
		result = fotahttp_recv(soc_id, buf, RECV_BLOCK_SIZE, 0);
		if (result >E_FOTA_SUCCESS)
		{
			if(HttpRecvNullTime > 0)
				HttpRecvNullTime = 0;
			res=Fotahttp_RecvPerform(s_settings,s_parser,buf,result);
			if (res<0)
			{
				Fota_printf("FOTA-->>>\r\n http parser error.\r\n"); 
				break;
			}
			else if(res==0)
			{
				Fota_printf("FOTA-->>>\r\n recv end and http parser succeed.\r\n"); 
				break;
			}

		}	
		else
		{
			if(E_FOTA_FAILURE== result)
			{
				if(HttpRecvNullTime >= trymaxnum)
				{
					HttpRecvNullTime = 0;
					FotaHttp_SetNetStatus(DL_NETWORK_ERROR);
					Fota_printf("FOTA-->>>\r\n\r\nhttp_recv failed!\r\n"); 
				}
				else
				{
					Fota_printf("FOTA-->>>\r\n\r\n http_recv block retry!\r\n"); 
					HttpRecvNullTime++;
					//adups_task_delay(1000);
					PlatTaskSleep(500);
					continue;
				}
			}	
			break;
		}
		//Osp_Delay_Task(1000);
    }

	Fotahttp_ResponsePost(s_settings,s_parser);
	fotahttp_soc_close(soc_id);
	FotaMain_Progress(fota_info, fota_state);

	Fota_printf("-------------fota_soc_recv  line=%d-----------------\r\n",__LINE__); 
}


void  fotahttp_requestprocess(fota_socint soc_id,FotaState fota_state,fota_int32 trymaxnum)
{
   	Request *req = 0;
   	fota_int32 result = 0;

   	req = make_request(soc_id);
	
   	dump_data((fota_uint8 *)req->header.data,fota_strlen(req->header.data));
   	//dump_data((fota_uint8 *)req->body.data,adups_strlen(req->body.data));
   	//Fota_printf(  "\r\nfota_soc_send  len1=%d,len2=%d\r\n", adups_strlen(req->header.data),adups_strlen(req->body.data));

	if(req == NULL)
	{
		//Fota_printf("FOTA-->>>fota_soc_send   SOC_ERROR \r\n"); 
		fotahttp_soc_close(soc_id);
		FotaHttp_SetNetStatus(DL_NETWORK_ERROR);
		FotaError_Progress();
		return;
	}
	result=fotahttp_send(soc_id,req);

   	free_request(req);
	//Fota_printf("FOTA-->>>\r\nfota_soc_send   result=%d fota_state = %d \r\n",result,fota_state); 

   	if (result == E_FOTA_SUCCESS){

		fotahttp_getresponse(soc_id,trymaxnum);
	}
    else if(result == E_FOTA_FAILURE)
    {
		//Fota_printf("FOTA-->>>fota_soc_send   SOC_ERROR \r\n"); 
		fotahttp_soc_close(soc_id);
		FotaHttp_SetNetStatus(DL_NETWORK_ERROR);
		FotaError_Progress();
    }
}














