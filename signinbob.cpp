#include "signinbob.h"

#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

struct InternetContext{
	HANDLE hConnectedEvent;
	HANDLE hCompletedEvent;
	//...
};
InternetContext context={0};
struct InternetNotifyHandler{
	DWORD dwInternetStatus;
	DWORD(*InternetNotify)(DWORD dwContext,LPVOID lpStatusInfo,DWORD dwStatusInfoLen);
};
DWORD DefaultInternetNotify(DWORD dwContext,LPVOID lpStatusInfo,DWORD dwStatusInfoLen)
{
	return e_ok;
}
DWORD InternetNotifyForConnected(DWORD dwContext,LPVOID lpStatusInfo,DWORD dwStatusInfoLen)
{
	SetEvent(context.hConnectedEvent);
	return e_ok;
}
DWORD InternetNotifyForCompleted(DWORD dwContext,LPVOID lpStatusInfo,DWORD dwStatusInfoLen)
{
	SetEvent(context.hCompletedEvent);
	return e_ok;
}
InternetNotifyHandler handlers[] = {
	{INTERNET_STATUS_CLOSING_CONNECTION,DefaultInternetNotify},
	{INTERNET_STATUS_CONNECTED_TO_SERVER,InternetNotifyForConnected},
	{INTERNET_STATUS_CONNECTING_TO_SERVER,DefaultInternetNotify},
	{INTERNET_STATUS_CONNECTION_CLOSED,DefaultInternetNotify},
	{INTERNET_STATUS_CTL_RESPONSE_RECEIVED,DefaultInternetNotify},
	{INTERNET_STATUS_HANDLE_CLOSING,DefaultInternetNotify},
	{INTERNET_STATUS_HANDLE_CREATED,DefaultInternetNotify},
	{INTERNET_STATUS_INTERMEDIATE_RESPONSE,DefaultInternetNotify},
	{INTERNET_STATUS_NAME_RESOLVED,DefaultInternetNotify},
	{INTERNET_STATUS_PREFETCH,DefaultInternetNotify},
	{INTERNET_STATUS_RECEIVING_RESPONSE,DefaultInternetNotify},
	{INTERNET_STATUS_REDIRECT,DefaultInternetNotify},

	{INTERNET_STATUS_REQUEST_COMPLETE,InternetNotifyForCompleted},
	{INTERNET_STATUS_REQUEST_SENT,DefaultInternetNotify},
	{INTERNET_STATUS_RESOLVING_NAME,DefaultInternetNotify},
	{INTERNET_STATUS_RESPONSE_RECEIVED,DefaultInternetNotify},
	{INTERNET_STATUS_SENDING_REQUEST,DefaultInternetNotify},
	{INTERNET_STATUS_STATE_CHANGE,DefaultInternetNotify},
};

static void __stdcall InternetNotify(HINTERNET hInternet,
              DWORD dwContext,
              DWORD dwInternetStatus,
              LPVOID lpStatusInfo,
              DWORD dwStatusInfoLen)
{
	SIGNIN_P("%d",dwInternetStatus);
	for(int i=0;i<sizeof(handlers)/sizeof(handlers[0]);i++)
	{
		if(handlers[i].dwInternetStatus==dwInternetStatus)
			handlers[i].InternetNotify(dwContext,lpStatusInfo,dwStatusInfoLen);
	}
	SIGNIN_P("returned?");

}
signinbob_ret c_signinbob::signin()
{
	HINTERNET hInternet=NULL;
	HINTERNET hConnect=NULL;
	HINTERNET hReq=NULL;
	INTERNET_STATUS_CALLBACK notify=NULL;
	char* headers="Accept: */*\r\n"\
                  "Accept-Language: zh-cn\r\n"\
                  "Content-Type: application/x-www-form-urlencoded\r\n\r\n"; 
	SIGNIN_P("signin begin\n");

	/* wait object */
	context.hConnectedEvent = CreateEvent(NULL,0,0,"Connected");
	context.hCompletedEvent = CreateEvent(NULL,0,0,"Completed");

	hInternet = InternetOpen("",
				INTERNET_OPEN_TYPE_DIRECT,
				NULL,
				NULL,
				INTERNET_FLAG_ASYNC
				);
	if(hInternet==NULL){
		SIGNIN_P("wininet open error\n");
		return e_open;
	}
	notify=InternetSetStatusCallback(hInternet,InternetNotify);
	if(notify==INTERNET_INVALID_STATUS_CALLBACK){
		SIGNIN_P("set notify callback error\n");
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return e_set_notify;
	}
	hConnect = InternetConnect(hInternet,"www.jd.com",80,NULL,NULL,INTERNET_SERVICE_HTTP,NULL,(DWORD)&context/*context*/);
	if(hConnect==NULL){
		if(GetLastError()!=ERROR_IO_PENDING)
		{
			SIGNIN_P("connect error\n");
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_connect;
		}
		if(WaitForSingleObject(context.hConnectedEvent,INFINITE)!=WAIT_OBJECT_0)
		{
			SIGNIN_P("timeout error\n");
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_timeout;
		}
		return e_timeout;
	}
	hReq = HttpOpenRequest(hConnect,"GET","/","HTTP/1.1",NULL,NULL,INTERNET_FLAG_DONT_CACHE,(DWORD)&context/*context*/);
	if(hReq==NULL){
		if(GetLastError()!=ERROR_IO_PENDING)
		{
			SIGNIN_P("open request error\n");
			InternetCloseHandle(hReq);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_open_req;
		}
		if(WaitForSingleObject(context.hCompletedEvent,INFINITE)!=WAIT_OBJECT_0)
		{
			SIGNIN_P("timeout error\n");
			InternetCloseHandle(hReq);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_timeout;
		}
		return e_timeout;
	}
	bool ret = HttpSendRequest(hReq,headers,strlen(headers),"",0)==TRUE;
	if(ret==false){
		if(GetLastError()!=ERROR_IO_PENDING)
		{
			SIGNIN_P("send request error %d\n",GetLastError());
			InternetCloseHandle(hReq);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_send_req;
		}
		if(WaitForSingleObject(context.hCompletedEvent,INFINITE)!=WAIT_OBJECT_0)
		{
			SIGNIN_P("timeout error\n");
			InternetCloseHandle(hReq);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_timeout;
		}

	}
	InternetCloseHandle(hReq);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
	return e_ok;
}