#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "signinbob_log.h"

HANDLE SigninLogger::mutex = NULL;

int SigninLogger::Log(int lev,char* fmt,...){
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
int SigninLogger::D(char* fmt,...){
	WaitForSingleObject(mutex,INFINITE);
	int len = 0;
	char buf[1024]={0};
	if(Dbg<=level){
		va_list args;
		va_start(args,fmt);
		fprintf(stdout,"%s","[Dbg]");
		len += vfprintf(stdout,fmt,args);
		va_end(args);
	}
	ReleaseMutex(mutex);
	return len;
}
int SigninLogger::I(char* fmt,...){
	WaitForSingleObject(mutex,INFINITE);
	int len = 0;
	if(Info<=level){
		va_list args;
		va_start(args,fmt);
		len += fprintf(stdout,"[Info]");
		len += vfprintf(stdout,fmt,args);
		va_end(args);
	}
	ReleaseMutex(mutex);
	return len;
}
int SigninLogger::E(char* fmt,...){
	WaitForSingleObject(mutex,INFINITE);
	int len = 0;
	if(Err<=level){
		va_list args;
		va_start(args,fmt);
		len += fprintf(stdout,"[Err]");
		len = vfprintf(stdout,fmt,args);
		va_end(args);
	}
	ReleaseMutex(mutex);
	return len;
}