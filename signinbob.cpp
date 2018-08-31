#include "signinbob.h"
#include "signinbob_net.h"
#include "signinbob_assert.h"
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

HANDLE hCom;

VOID CALLBACK InternetStatusCallback(
			HINTERNET hInternet,DWORD dwContext,
			DWORD dwInternetStatus,LPVOID lpvStatusInfo,
			DWORD dwStatusInfoLen)
{
	SigninLogger logger;
	logger.SetLevel(SigninLogger::Dbg);
	logger.D("callback %d\n",dwInternetStatus);
	switch(dwInternetStatus)
	{
	case INTERNET_STATUS_REQUEST_COMPLETE:
		SetEvent(hCom);
		break;
	case INTERNET_STATUS_RESPONSE_RECEIVED:
		SetEvent(hCom);
		break;
	}
}

signinbob_ret c_signinbob::signin()
{
	SigninLogger logger;

	hCom = CreateEvent(NULL,0,0,"completed");
	logger.SetLevel(SigninLogger::Dbg);

	HINTERNET hInternet=NULL;
	HINTERNET hConnect=NULL;
	HINTERNET hReq=NULL;

	char* headers="Accept: */*\r\n"\
                  "Accept-Language: zh-cn\r\n"\
                  "Content-Type: application/x-www-form-urlencoded\r\n\r\n"; 

	logger.D("signin begin\n");

	hInternet = InternetOpen("",
				INTERNET_OPEN_TYPE_DIRECT,
				NULL,
				NULL,
				INTERNET_FLAG_ASYNC
				);
	if(hInternet==NULL){
		logger.D("wininet open error\n");
		return e_open;
	}
	logger.D("wininet open ok\n");

	hConnect = InternetConnect(hInternet,"baidu.com",80,NULL,NULL,INTERNET_SERVICE_HTTP,NULL,0);
	if(hConnect==NULL){
		if(GetLastError()!=ERROR_IO_PENDING)
		{
			logger.D("connect error\n");
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_connect;
		}
	}
	logger.D("connnect ok\n");
	InternetSetStatusCallback(hConnect,InternetStatusCallback);
	hReq = HttpOpenRequest(hConnect,"GET","/dfdfds","HTTP/1.1",NULL,NULL,INTERNET_FLAG_DONT_CACHE,1/*context*/);
	if(hReq==NULL){
		if(GetLastError()!=ERROR_IO_PENDING)
		{
			logger.D("open request error\n");
			InternetCloseHandle(hReq);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_open_req;
		}
		return e_timeout;
	}
	logger.D("open req ok\n");
	int ret = HttpAddRequestHeaders(hReq,headers,strlen(headers),HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);
	if(!ret)
		logger.D("add header err\n");
	else
		logger.D("add header ok\n");
	ret = HttpSendRequest(hReq,NULL,0,"",0);
	
	if(ret==0){
		logger.D("GetLastError %d\n",GetLastError());
		if(GetLastError()!=ERROR_IO_PENDING)
		{
			logger.D("send request error %d\n",GetLastError());
			InternetCloseHandle(hReq);
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			return e_send_req;
		}
		logger.D("222\n");
		WaitForSingleObject(hCom,INFINITE);
		logger.D("send ok \n");


	}else{
		logger.D("send ok\n");
	}
	DWORD dwStatusCode;
	DWORD dwSizeDW=sizeof(DWORD);
	if (!HttpQueryInfo(hReq, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &dwStatusCode, &dwSizeDW, NULL))
    {
		logger.D("query err\n");
		logger.D("err  %d \n",GetLastError());
	}else{
		logger.D("query ok %d\n",dwStatusCode);

	}
	char buf[1025]={0};
	int retlen = 0;
	InternetReadFile(hReq,buf,1024,(DWORD*)&retlen);
	logger.D("retlen = %s\n",buf);
	InternetCloseHandle(hReq);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
	logger.D("ooooo\n");
	return e_ok;
}