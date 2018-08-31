#ifndef _signinbob_log_h
#define _signinbob_log_h
#include <windows.h>

/*
 use mutex to synchronous write out
*/

class SigninLogger{
public:
	enum{
		Err,
		Info,
		Dbg
	};
	SigninLogger(){
		level=Err;
		if(mutex==NULL)
			mutex = CreateMutex(NULL,0/*owner*/,"lock");
	}
	void SetLevel(int lev)
	{
		level = lev;
	}
	int Log(int lev,char* fmt,...);
	int D(char* fmt,...);
	int I(char* fmt,...);
	int E(char* fmt,...);
private:
	char* LogLevelName(int lev){
		if(lev==Err)
			return "Err";
		if(lev==Info)
			return "info";
		if(lev==Dbg)
			return "Dbg";
		// others
		return "";
	}
	static HANDLE mutex;
	int level;
};


#endif
