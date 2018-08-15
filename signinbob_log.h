#ifndef _signinbob_log_h
#define _signinbob_log_h

#include <stdio.h>
#include <stdlib.h>

int signinbob_log(char* fmt,...);
inline int signinbob_nolog(char* fmt,...);

#ifndef NDEBUG
#	define SIGNIN_P do{fprintf(stdout,"(%s,%d)",__FILE__,__LINE__);}while(0);signinbob_log
#else
#	define SIGNIN_P signinbob_nolog
#endif

#endif
