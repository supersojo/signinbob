#include "signinbob.h"

#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

static void __stdcall InternetNotify(HINTERNET hInternet,
              DWORD dwContext,
              DWORD dwInternetStatus,
              LPVOID lpStatusInfo,
              DWORD dwStatusInfoLen)
{
	SIGNIN_P("%d",dwContext);
	switch(dwContext)
	{
		ssss
	}

}
signinbob_ret c_signinbob::signin()
{
	HANDLE hConnectedEvent=NULL,hCompletedEvent=NULL;
	HINTERNET hInternet=NULL;
	HINTERNET hConnect=NULL;
	HINTERNET hReq=NULL;
	INTERNET_STATUS_CALLBACK notify=NULL;
	char* headers="Accept: */*\r\n"\
                  "Accept-Language: zh-cn\r\n"\
                  "Content-Type: application/x-www-form-urlencoded\r\n\r\n"; 
	SIGNIN_P("signin begin\n");

	/* wait object */
	hConnectedEvent = CreateEvent(NULL,0,0,"Connected");
	hCompletedEvent = CreateEvent(NULL,0,0,"Completed");

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
	hConnect = InternetConnect(hInternet,"www.jd.com",80,NULL,NULL,INTERNET_SERVICE_HTTP,NULL,1/*context*/);
	if(hConnect==NULL){
		if(GetLastError()!=ERROR_IO_PENDING)
		{
			SIGNIN_P("connect error\n");
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_connect;
		}
		if(WaitForSingleObject(hConnectedEvent,INFINITE)!=WAIT_OBJECT_0)
		{
			SIGNIN_P("timeout error\n");
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_timeout;
		}
		return e_timeout;
	}
	hReq = HttpOpenRequest(hConnect,"GET","/","HTTP/1.1",NULL,NULL,INTERNET_FLAG_DONT_CACHE,2/*context*/);
	if(hReq==NULL){
		if(GetLastError()!=ERROR_IO_PENDING)
		{
			SIGNIN_P("open request error\n");
			InternetCloseHandle(hReq);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_open_req;
		}
		if(WaitForSingleObject(hCompletedEvent,INFINITE)!=WAIT_OBJECT_0)
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
		if(WaitForSingleObject(hCompletedEvent,INFINITE)!=WAIT_OBJECT_0)
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