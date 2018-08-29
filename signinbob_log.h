#ifndef _signinbob_log_h
#define _signinbob_log_h

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

class SigninLogger{
public:
	enum{
		Err,
		Info,
		Dbg
	};
	SigninLogger(){
		level=Err;
	}
	void SetLevel(int lev)
	{
		level = lev;
	}
	int Log(int lev,char* fmt,...){
		int len = 0;
		if(lev<=level){
			va_list args;
			va_start(args,fmt);
			len += fprintf(stdout,"%s",LogLevelName(lev));
			len += vfprintf(stdout,fmt,args);
			va_end(args);
		}
		return len;
	}
	int D(char* fmt,...){
		int len = 0;
		if(Dbg<=level){
			va_list args;
			va_start(args,fmt);
			len += fprintf(stdout,"[Dbg]");
			len += vfprintf(stdout,fmt,args);
			va_end(args);
		}
		return len;
	}
	int I(char* fmt,...){
		int len = 0;
		if(Info<=level){
			va_list args;
			va_start(args,fmt);
			len += fprintf(stdout,"[Info]");
			len += vfprintf(stdout,fmt,args);
			va_end(args);
		}
		return len;
	}
	int E(char* fmt,...){
		int len = 0;
		if(Err<=level){
			va_list args;
			va_start(args,fmt);
			len += fprintf(stdout,"[Err]");
			len = vfprintf(stdout,fmt,args);
			va_end(args);
		}
		return len;
	}
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
	int level;
};
int signinbob_log(char* fmt,...);
inline int signinbob_nolog(char* fmt,...);

#ifndef NDEBUG
#	define SIGNIN_P do{fprintf(stdout,"(%s,%d)",__FILE__,__LINE__);}while(0);signinbob_log
#else
#	define SIGNIN_P signinbob_nolog
#endif

#endif
