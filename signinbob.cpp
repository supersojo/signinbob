#include "signinbob.h"
#include "signinbob_net.h"
#include "signinbob_assert.h"
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")



signinbob_ret c_signinbob::signin()
{
	HINTERNET hInternet=NULL;
	HINTERNET hConnect=NULL;
	HINTERNET hReq=NULL;

	char* headers="Accept: */*\r\n"\
                  "Accept-Language: zh-cn\r\n"\
                  "Content-Type: application/x-www-form-urlencoded\r\n\r\n"; 
	SIGNIN_P("signin begin\n");

	/* wait object */

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
	SIGNIN_P("dddddd\n");
	InternetContextBase* context = new InternetContextBase(CreateEvent(NULL,false,false,"connected"),CreateEvent(NULL,false,false,"completed"));
	WininetCallbackBase* callback = new WininetCallbackBase(hInternet,context);
	callback->RegisterInternetStatusCallback();
	SIGNIN_P("fffff\n");
	hConnect = InternetConnect(hInternet,"www.jd.com",80,NULL,NULL,INTERNET_SERVICE_HTTP,NULL,(DWORD)callback/*context*/);
	if(hConnect==NULL){
		if(GetLastError()!=ERROR_IO_PENDING)
		{
			SIGNIN_P("connect error\n");
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_connect;
		}
		if(WaitForSingleObject(context->hConnectedEvent,INFINITE)!=WAIT_OBJECT_0)
		{
			SIGNIN_P("timeout error\n");
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_timeout;
		}
		return e_timeout;
	}
	SIGNIN_P("ggggg %x\n",callback);
	hReq = HttpOpenRequest(hConnect,"GET","/","HTTP/1.1",NULL,NULL,INTERNET_FLAG_DONT_CACHE,(DWORD)&callback/*context*/);
	if(hReq==NULL){
		if(GetLastError()!=ERROR_IO_PENDING)
		{
			SIGNIN_P("open request error\n");
			InternetCloseHandle(hReq);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_open_req;
		}
		if(WaitForSingleObject(context->hCompletedEvent,INFINITE)!=WAIT_OBJECT_0)
		{
			SIGNIN_P("timeout error\n");
			InternetCloseHandle(hReq);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_timeout;
		}
		return e_timeout;
	}
	SIGNIN_P("hhhh\n");
	bool ret = HttpSendRequest(hReq,headers,strlen(headers),"",0)==TRUE;
	SIGNIN_P("1111\n");
	if(ret==false){
		if(GetLastError()!=ERROR_IO_PENDING)
		{
			SIGNIN_P("send request error %d\n",GetLastError());
			InternetCloseHandle(hReq);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_send_req;
		}
		SIGNIN_P("222\n");
		if(WaitForSingleObject(context->hCompletedEvent,INFINITE)!=WAIT_OBJECT_0)
		{
			SIGNIN_P("timeout error\n");
			InternetCloseHandle(hReq);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_timeout;
		}

	}
	SIGNIN_P("pppp\n");
	InternetCloseHandle(hReq);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
	SIGNIN_P("ooooo\n");
	return e_ok;
}