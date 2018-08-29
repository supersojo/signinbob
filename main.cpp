/*
 * main.cpp
 */
#include "signinbob.h"
#include "signinbob_assert.h"

int main(int argc, char* argv[])
{
	SIGNIN_ASSERT(1>0,"1<0");
	//c_signinbob::signin();
	SigninLogger logger;
	return 0;
}