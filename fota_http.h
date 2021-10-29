#ifndef _FOTA_HTTP_H_
#define _FOTA_HTTP_H_

typedef struct
{
    UINT16 nYear;
    UINT8 nMonth;
    UINT8 nDay;
    UINT8 nHour;
    UINT8 nMin;
    UINT8 nSec;
    UINT8 DayIndex; /* 0=Sunday */
} Fota_time_struct;

typedef struct _FotaVersionInfo {
	INT32  status;
	char *msg;
	char *versionName;
	INT32   fileSize;
	char *deltaID;
	char *md5sum;
	char *deltaUrl;
	char *publishDate;
	char *releaseContent;
	INT32  storageSize;
	INT32 	battery;
	INT32  	download_status;
	UINT32 download_start;
	UINT32 download_end;
	INT32  update_status;
	char *deviceSecret;
	char *deviceId;
} asrFotaVersionInfo;


typedef struct _FotaMobileParamInfo {
	char  *mid;
	char  *version;
	char  *oem;
	char  *models;
	char  *productid;
	char  *productsec;	
	char  *platform;
	char  *device_type;
	char  *mac;
} FotaMobileParamInfo;

typedef struct _FotaDownParamInfo {
	char  *mid;
	char  *token;
	char  *deltaID;
	char  *productid;
	char  *productsec;	
	INT32         download_status;
	INT32      download_start;
	INT32      download_end;
	UINT32 download_size;
	char *download_ip;
} FotaDownParamInfo;

typedef struct _FotaUpgradeParamInfo {
	char  *mid;
	char  *token;
	char  *deltaID;
	char  *productid;
	char  *productsec;	
	INT32  update_status;
	char   *extendString;
} FotaUpgradeParamInfo;



typedef struct _httpdatabuf {
    char        *data;
    INT32       limit;
    INT32       size;
} httpdatabuf;








int fota_connect_to_server(UINT8* host,UINT16 port);

int PdnIsconnected(void);

void  fotahttp_requestprocess(fota_socint soc_id,FotaState fota_state,fota_int32 trymaxnum);

int Fotahttp_RecvPerform(http_parser_settings *p_settings,http_parser *p_parser,const char *buf,size_t len);

void Fotahttp_ResponseSetting(http_parser_settings *p_settings,http_parser *p_parser);						
void FotaHttp_SetNetStatus(int network_status);
void Fotahttp_ResponsePost(http_parser_settings *p_settings,http_parser *p_parser);
extern Request *make_request(fota_socint soc_id);
extern void free_request(Request *req);
void FotaError_Progress(void);
void FotaMain_Progress(VersionInfo *info, fota_int32 state);

#ifdef MINGW64
#define  socketclose closesocket
#endif







#endif

