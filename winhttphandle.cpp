#include "winhttphandle.h"
#include <stdio.h>
#pragma  comment(lib, "winhttp.lib")

HINTERNET WinHttpHandle::GetHandle()const
{
	return m_handle;
}

WinHttpHandle::~WinHttpHandle()
{
	Close();
}

void WinHttpHandle::Close()
{
	if(m_handle)
		::WinHttpCloseHandle(m_handle);

	m_handle = 0;
}

bool WinHttpHandle::Attach(HINTERNET h)
{
	if(m_handle!=0)
		return false;
	if(h==0)
		return false;
	m_handle = h;
		return true;
}

HINTERNET WinHttpHandle::Detach()
{
	HINTERNET h = m_handle;
	m_handle  = 0;
	return h;
}

HRESULT WinHttpHandle::SetOption(DWORD option,const void* value,DWORD len)
{
	if(false==::WinHttpSetOption(m_handle,option,(LPVOID)(value),len))
	{
		return HRESULT_FROM_WIN32(::GetLastError());
	}
	return S_OK;
}

HRESULT WinHttpHandle::QueryOption(DWORD option,void* value,DWORD &len)const
{
	if(false==::WinHttpQueryOption(m_handle,option,value,&len))
	{
		return HRESULT_FROM_WIN32(::GetLastError());
	}
	return S_OK;
}

HRESULT WinHttpSession::Initialize()
{
	HINTERNET h;
#if 1
	h = ::WinHttpOpen(L"",
			WINHTTP_ACCESS_TYPE_NAMED_PROXY,
			L"10.167.196.133:8080",
			NULL,
			WINHTTP_FLAG_ASYNC
			);
#else
	h = ::WinHttpOpen(0,/* User-Agent */
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		WINHTTP_FLAG_ASYNC);
#endif
	if (!Attach(h)){
		return HRESULT_FROM_WIN32(::GetLastError());
	}
	return S_OK;
}

HRESULT WinHttpConnection::Initialize(PCWSTR server,INTERNET_PORT port,const WinHttpSession& session)
{
	HINTERNET h;
	h = ::WinHttpConnect(session.GetHandle(),
		server,
		port,
		0);
	if (!Attach(h)){
		return HRESULT_FROM_WIN32(::GetLastError());
	}
	return S_OK;
}

SimpleBuffer::SimpleBuffer()
{
	m_data = new char[1024+1];//additional "\0"
	m_len = 1024;
}
char* SimpleBuffer::GetData()
{
	return m_data;
}
int SimpleBuffer::GetLen()
{
	return m_len;
}
SimpleBuffer::~SimpleBuffer()
{
	delete[] m_data;
	m_data = 0;
	m_len = 0;
}
HRESULT WinHttpRequest::Initialize(PCWSTR path,PCWSTR verb,const WinHttpConnection& connnection)
{
	HINTERNET h;
	h = ::WinHttpOpenRequest(connnection.GetHandle(),
		verb,
		path,
		L"HTTP/1.1",
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		0);
	if(!Attach(h))
		return HRESULT_FROM_WIN32(::GetLastError());

	if(WINHTTP_INVALID_STATUS_CALLBACK==::WinHttpSetStatusCallback(GetHandle(),Callback,WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,0))
		return HRESULT_FROM_WIN32(::GetLastError());

	return S_OK;
}

HRESULT WinHttpRequest::SendRequest(PCWSTR headers,DWORD hlen,const void*opt,DWORD optlen,DWORD total)
{
	if(!::WinHttpSendRequest(GetHandle(),headers,hlen,(LPVOID)opt,optlen,total,(DWORD_PTR)this))
		return HRESULT_FROM_WIN32(::GetLastError());

	return S_OK;
}

void CALLBACK WinHttpRequest::Callback(HINTERNET handle,
									   DWORD_PTR context,
									   DWORD code,
									   void* info,
									   DWORD length)
{
	if(context)
	{
		WinHttpRequest* pt = reinterpret_cast<WinHttpRequest*>(context);
		HRESULT result = pt->OnCallback(code,info,length);
		if(FAILED(result))
			pt->OnResponseComplete(result);
	}
}

HRESULT WinHttpRequest::OnCallback(DWORD code,void* info,DWORD length)
{
	switch(code)
	{
	case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
	case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:
	{
		HRESULT result;
		if(code==WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE)
			result = OnWriteData(0);
		else
			result = OnWriteData(*((DWORD*)info));
		if(FAILED(result))
			return result;

		if(S_FALSE==result)
		{
			if(!::WinHttpReceiveResponse(GetHandle(),0))
				return HRESULT_FROM_WIN32(::GetLastError());
		}
		break;
	}
	case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
	{
		DWORD statusCode = 0;
		DWORD statusCodeSize = sizeof(DWORD);
		if (!::WinHttpQueryHeaders(GetHandle(),
			WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			WINHTTP_HEADER_NAME_BY_INDEX,
			&statusCode,
			&statusCodeSize,
			WINHTTP_NO_HEADER_INDEX))
		{
			return HRESULT_FROM_WIN32(::GetLastError());
		}
		if (HTTP_STATUS_OK != statusCode)
		{
			return E_FAIL;
		}

		// receive data
		if(!::WinHttpReadData(GetHandle(),m_buf.GetData(),m_buf.GetLen(),0))
			return HRESULT_FROM_WIN32(::GetLastError());
		break;
	}
	case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
		printf("error\n");
		break;
	case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
		if(length>0)
		{
			OnReadComplete(m_buf.GetData(),length);
			// receive data
			if(!::WinHttpReadData(GetHandle(),m_buf.GetData(),m_buf.GetLen(),0))
				return HRESULT_FROM_WIN32(::GetLastError());
		}
		else
			OnResponseComplete(S_OK);
		break;
	}
	return S_OK;
}

void WinHttpRequest::OnResponseComplete(HRESULT result)
{
}

HRESULT WinHttpRequest::OnReadComplete(char* data,int len)
{
	*(data+len)='\0';
	//printf("%s",data);
	return S_OK;
}

HRESULT WinHttpRequest::OnWriteData(int len)
{
	return S_FALSE;
}

HRESULT DownloadFileRequest::Initialize(PCWSTR path,const WinHttpConnection& connection)
{
	m_hc = ::CreateEvent(NULL,0,0,""); 
	return WinHttpRequest::Initialize(path,L"GET",connection);
}

HRESULT DownloadFileRequest::OnReadComplete(char* data,int len)
{
	*(data+len)='\0';
	//printf("%s",data);
	return S_OK;
}

void DownloadFileRequest::OnResponseComplete(HRESULT result)
{
	m_result = result;
	SetEvent(m_hc);

	if(result==S_OK)
	{
	}
}

HRESULT DownloadFileRequest::Wait()
{
	::WaitForSingleObject(m_hc,5*1000);
	return m_result;
}

HRESULT PostDataRequest::Initialize(PCWSTR path,const WinHttpConnection& connection)
{
	count = 6;
	m_hc = ::CreateEvent(NULL,0,0,""); 
	return WinHttpRequest::Initialize(path,L"POST",connection);
}

HRESULT PostDataRequest::OnReadComplete(char* data,int len)
{
	*(data+len)='\0';
	//printf("%s\n",data);
	return S_OK;
}

void PostDataRequest::OnResponseComplete(HRESULT result)
{
	m_result = result;
	SetEvent(m_hc);

	if(result==S_OK)
	{
	}
}

HRESULT PostDataRequest::OnWriteData(int len)
{

	count-=len;
	if(count>0)
	{

		if(!::WinHttpWriteData(GetHandle(),"H=h",3,0)){

			return HRESULT_FROM_WIN32(::GetLastError());
		}

		return S_OK;
	}

	return S_FALSE;
}

HRESULT PostDataRequest::Wait()
{
	::WaitForSingleObject(m_hc,5*1000);
	return m_result;
}

int main()
{
	WinHttpSession  ses;
	if(FAILED(ses.Initialize())){
		printf("ses failed\n");
		return -1;
	}
	WinHttpConnection con;
	if(FAILED(con.Initialize(L"www.baidu.com",80,ses)))
	{
		printf("con failed\n");
		return -1;
	}

	while(1){
		PostDataRequest req;
		if(FAILED(req.Initialize(L"/",con)))
		{
			printf("req failed\n");
			return -1;
		}
		if(FAILED(req.SendRequest(NULL,0,NULL,0,0)))
			printf("send req failed\n");
		else
			printf("send ok\n");
		HRESULT res = req.Wait();
		printf("result=%d\n",res);
		Sleep(1000);
	}
#if 0
	while(1)
	{
		DownloadFileRequest req;
		if(FAILED(req.Initialize(L"/",con)))
		{
			printf("req failed\n");
			return -1;
		}
		if(FAILED(req.SendRequest(NULL,0,NULL,0,0)))
			printf("send req failed\n");
		else
			printf("send ok\n");
		HRESULT res = req.Wait();
		printf("result=%d\n",res);

		Sleep(1000);
	}
#endif
}
