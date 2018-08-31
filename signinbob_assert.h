#ifndef _signinbob_assert_h
#define _signinbob_assert_h

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define SIGNIN_ASSERT(condition,msg) \
do{ \
	if(!(condition)){ \
		fprintf(stdout,"Assertion failed in file %s in line %d",__FILE__,__LINE__); \
		exit(-1); \
	} \
}while(0)

#endif