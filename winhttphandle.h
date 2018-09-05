#include <windows.h>

#ifndef DWORD_PTR
typedef ULONG_PTR DWORD_PTR;
#endif
#include <winhttp.h>

class WinHttpHandle
{
public:
	WinHttpHandle():m_handle(0){}

	bool Attach(HINTERNET h);

	HINTERNET Detach();

	HRESULT SetOption(DWORD option,const void* value,DWORD len);

	HRESULT QueryOption(DWORD option,void* value,DWORD& len)const;
	
	HINTERNET GetHandle()const;

	~WinHttpHandle();
	
private:
	void Close();

	HINTERNET m_handle;
};

class WinHttpSession : public WinHttpHandle
{
public:
	HRESULT Initialize();
};

class WinHttpConnection : public WinHttpHandle
{
public:
	HRESULT Initialize(PCWSTR server,INTERNET_PORT port,const WinHttpSession& session);
};

class SimpleBuffer
{
public:
	SimpleBuffer();
	char* GetData();
	int GetLen();
	~SimpleBuffer();
private:
	char* m_data;
	int m_len;
};

class WinHttpRequest : public WinHttpHandle
{
public:
	HRESULT Initialize(PCWSTR path,PCWSTR verb,const WinHttpConnection& connnection);
	HRESULT SendRequest(PCWSTR headers,DWORD hlen,const void*opt,DWORD optlen,DWORD total);
private:
	SimpleBuffer m_buf;
protected:
	HRESULT OnCallback(DWORD code,void* info,DWORD length);
	virtual HRESULT OnReadComplete(char* data,int len);
	virtual void OnResponseComplete(HRESULT result);
	static void CALLBACK Callback(HINTERNET handle,
		DWORD_PTR context,
		DWORD code,
		void* info,
		DWORD length);
};

class DownloadFileRequest : public WinHttpRequest
{
public:
	DownloadFileRequest(){m_hc=0;}
	HRESULT Initialize(PCWSTR path,const WinHttpConnection& connection);
	virtual void OnResponseComplete(HRESULT result);
	virtual HRESULT OnReadComplete(char* data,int len);
	HRESULT Wait();
private:
	HANDLE m_hc;
	HRESULT m_result;
};