#ifndef _FOTACOMM_TYPEDEF_H_
#define _FOTACOMM_TYPEDEF_H_

#ifndef fotacomm_extern
#ifdef __cplusplus
#define fotacomm_extern extern "C"
#else
#define fotacomm_extern extern
#endif
#endif//fotacomm_extern

typedef char                    fota_char;

typedef unsigned short          fota_wchar;

typedef unsigned char           fota_uint8;

typedef signed char             fota_int8;

typedef unsigned short int      fota_uint16;

typedef signed short int        fota_int16;

typedef unsigned int            fota_uint32;

typedef signed int              fota_int32;


typedef unsigned long long   fota_uint64;

typedef signed long long     fota_int64;

typedef signed int              INT32;
typedef unsigned int            UINT32;


typedef   int   fota_socint;

#define PNULL NULL


typedef enum 
{
    FOTA_FALSE,
    FOTA_TRUE
} FOTA_BOOL;


typedef enum {
	STATE_INIT = 1,    
	STATE_CV,    
	STATE_DL,    
	STATE_RD,    
	STATE_UG,
	STATE_RU,
	STATE_RG
}FotaState;

typedef enum
{
	FOTA_SUCCESS=1000,
	FOTA_PID_ERROR,
	FOTA_PROJECT_ERROR,
	FOTA_PARAM_INVAILD,
	FOTA_PARAM_LOST,
	FOTA_SYS_ERROR,
	FOTA_JSON_ERROR,
	FOTA_RG_SIGN_ERROR=2001,
	FOTA_CV_LAST_VERSION=2101,
	FOTA_CV_INVAILD_VERSION,
	FOTA_CV_UNREG_DEVICE,
	FOTA_DL_STATE_ERROR=2201,
	FOTA_DL_DELTAID_ERROR,
	FOTA_RP_DELTAID_ERROR=2301,
	FOTA_RP_UPGRADE_STATE_ERROR	
}FOTACODE;

#define DL_SUCCESS FOTA_DL_SUCCESS
enum DownloadStatus {
	DL_SUCCESS = 1,  
	DL_NO_NETWORK,
	DL_CANCELED,
	DL_CACHE_READ_ERROR,
	DL_SERVER_ERROR,    
	DL_TIMEOUT, 
	DL_NETWORK_ERROR,
	DL_MD5_MIS_MATCH,
	DL_TIMER_CREAT_ERROR,
	DL_INIT_FLASH_ERROR,
	DL_FILESIZE_TOO_LARGE,
	DL_NO_SPACE,
	DL_PATH_NOT_EXIST,
	DL_ERRORS=99,
};



enum UpgradeStatus {
	UP_SUCCESS = 3000,    
	UP_UNZIP_ERROR,    
	UP_MD5_FAIL,    
	UP_NO_SPACE,    
	UP_WRITE_FILE_ERROR
};

typedef enum 
{
	CK_NEW_VERSION = 1000,    
	CK_PRODUCT_ID_INVALID,    
	CK_PROJECT_DOES_NOT_EXIST,    
	CK_PARAM_INVALID,   
	CK_PARAM_NEED_MORE, 
	CK_SYSTEM_ERROR,
	CK_JSON_PARSE_ERROR,
	CK_PARAM_ERROR,
	CK_MID_LENGTH_ERROR,
	CK_SIGN_ERROR=2001,
	CK_LAST_VERSION=2101,
	CK_END
}CKRESULT;

enum NetStatusMachine {
	NET_STAT_SUCCESS = 2000,    
	NET_STAT_DNS_REQ,		//2001
	NET_STAT_DNS_GETTING,	 
	NET_STAT_DNS_SUCC,	 
	NET_STAT_NEWSOC_REQ,    //2004
	NET_STAT_NEWSOC_SUCC,	
	NET_STAT_NEWSOC_ERR,    
	NET_STAT_DNS_ERR,
	NET_STAT_CONN_REQ,		//2008
	NET_STAT_CONNECTING,
	NET_STAT_CONN_SUCC,
	NET_STAT_CONN_ERR,
	NET_STAT_SEND_REQ,		//2012
	NET_STAT_SENDING,
	NET_STAT_SEND_SUCC,
	NET_STAT_SEND_ERR,
	NET_STAT_RECV_REQ,		//2016
	NET_STAT_RECVING,
	NET_STAT_RECV_SUCC,
	NET_STAT_RECV_SUCC2,
	NET_STAT_RECV_ERR,
	NET_STAT_CLOSE_REQ,		//2021
	NET_STAT_CLOSE_SUCC,
	NET_STAT_CLOSE_ERR
};

typedef struct
{
    fota_uint16 nYear;
    fota_uint8 nMonth;
    fota_uint8 nDay;
    fota_uint8 nHour;
    fota_uint8 nMin;
    fota_uint8 nSec;
    fota_uint8 DayIndex; /* 0=Sunday */
} Adups_time_struct;

typedef struct _VersionInfo {
	fota_int32  status;
	fota_char *msg;
	fota_char *versionName;
	fota_int32   fileSize;
	fota_char *deltaID;
	fota_char *md5sum;
	fota_char *deltaUrl;
	fota_char *publishDate;
	fota_char *releaseContent;
	fota_int32  storageSize;
	fota_int32 	battery;
	fota_int32  	download_status;
	fota_uint32 download_start;
	fota_uint32 download_end;
	fota_int32  update_status;
	fota_char *deviceSecret;
	fota_char *deviceId;
} VersionInfo;


typedef struct _MobileParamInfo {
	fota_char  *mid;
	fota_char  *version;
	fota_char  *oem;
	fota_char  *models;
	fota_char  *productid;
	fota_char  *productsec;	
	fota_char  *platform;
	fota_char  *device_type;
	fota_char  *mac;
} MobileParamInfo;

typedef struct _DownParamInfo {
	fota_char  *mid;
	fota_char  *token;
	fota_char  *deltaID;
	fota_char  *productid;
	fota_char  *productsec;	
	fota_int32         download_status;
	fota_int32      download_start;
	fota_int32      download_end;
	fota_uint32 download_size;
	fota_char *download_ip;
} DownParamInfo;

typedef struct _UpgradeParamInfo {
	fota_char  *mid;
	fota_char  *token;
	fota_char  *deltaID;
	fota_char  *productid;
	fota_char  *productsec;	
	fota_int32  update_status;
	fota_char   *extendString;
} UpgradeParamInfo;



typedef struct _streambuf {
    fota_char        *data;
    fota_int32         limit;
    fota_int32         size;
} streambuf;

typedef struct _request {
    streambuf   header;
    streambuf   body;
} Request;



#ifndef NULL
#define NULL  (void *)0           /*  NULL    :   Null pointer */
#endif


#define E_FOTA_SUCCESS			0
#define E_FOTA_FAILURE			-1
#define E_FOTA_NOMEMORY			-2
#define E_FOTA_NOTEXIST			-3
#define E_FOTA_WOULDBLOCK		-4
#define E_FOTA_SIM_NOTEXIST		-5
#define E_FOTA_MID_ERROR		-6
#define E_FOTA_NETWORK_ERROR		-7
#define E_FOTA_DOWNLOAD_FAIL		-8
#define E_FOTA_LAST_VERSION		-9
#define E_FOTA_TOKEN_WORNG		-10
#define E_FOTA_PROJECT_WORONG		-11
#define E_FOTA_PARAM_WORONG		-12
#define E_FOTA_NET_CONNECTING	-13
#define E_FOTA_DOWNLOAD_CONNECTING		-14


#define FOTA_MAX_DEBUG_BUF_LEN		256

#define RECV_BLOCK_SIZE (1024*4) //can not over the 4K limit

typedef void (*af_callback)(VersionInfo *info, fota_int32 state);

typedef void (*fota_notify)(int data);

typedef void (*fota_timer_callback)(void);


typedef enum 
{
	ADUPS_READ_NOTIFY,
	ADUPS_WRITE_NOTIFY,
	ADUPS_CLOSE_NOTIFY,
	ADUPS_ERROR_NOTIFY,
	ADUPS_SOCMGR_END
} ADUPS_SOCMGR_USR;

typedef enum 
{
	ADUPS_DIFF_PATCH,
	ADUPS_FULL_PATCH,
	ADUPS_PATCH_END
} ADUPS_PATCH_METHOD;

typedef struct 
{
	fota_notify fota_read_notify;
	fota_notify fota_write_notify;
	fota_notify fota_close_notify;
	fota_notify fota_net_error_notify;
	fota_int32 fota_soc_state;
	fota_int32 fota_request_id;
	fota_int32 fota_tcp_socket_id;
	fota_int32 fota_net_sm;

} FOTA_SOCKET_MGR_STRUCT;


typedef enum 
{
	ADUPS_READONLY	= 0x0001,
	ADUPS_READWRITE	= 0x0002,
	ADUPS_CREATE 	= 0x0004,
	ADUPS_APPEND 	= 0x0008,
	ADUPS_TRUNCATE = 0x0010
}ADUPS_OpenMode;



typedef struct
{
	fota_uint8 timerid;
	fota_uint8 timermode;
	fota_uint32 delay;
	fota_timer_callback func_ptr;
	void *timer_ptr;
}ADUPS_TIMER_STRUCT;


#endif

