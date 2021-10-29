#ifndef _FOTA_MAIN_H_
#define _FOTA_MAIN_H_



int fota_connect_to_server(UINT8* host,UINT16 port);

int PdnIsconnected(void);
void fotadevice_register(void);

void  fotahttp_requestprocess(fota_socint soc_id,FotaState fota_state,fota_int32 trymaxnum);

int Fotahttp_RecvPerform(http_parser_settings *p_settings,http_parser *p_parser,const char *buf,size_t len);

void Fotahttp_ResponseSetting(http_parser_settings *p_settings,http_parser *p_parser);						
void FotaHttp_SetNetStatus(int network_status);
void Fotahttp_ResponsePost(http_parser_settings *p_settings,http_parser *p_parser);
extern Request *make_request(fota_socint soc_id);
extern void free_request(Request *req);
void FotaError_Progress(void);
void FotaMain_Progress(VersionInfo *info, fota_int32 state);

void FotaParams_init(void);







#ifdef MINGW64
#define  socketclose closesocket
#endif







#endif

