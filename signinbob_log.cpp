#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int signinbob_log(char* fmt,...)
{
	va_list args;
	va_start(args,fmt);
	vfprintf(stdout,fmt,args);
	va_end(args);
	return 0;
}

int signinbob_nolog(char* fmt,...)
{
	return 0;
}