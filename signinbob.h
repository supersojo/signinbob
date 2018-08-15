#ifndef _signinbob_h
#define _signinbob_h

//#define NDEBUG

#include "signinbob_log.h"

enum _signinbob_ret{
	e_ok = 0,
	e_open = -1,
	e_connect = -2,
	e_open_req = -3,
	e_send_req = -4,
	e_timeout=-5,
	e_set_notify = -6,
	e_max
};

typedef enum _signinbob_ret signinbob_ret;

class c_signinbob
{
public:
	static signinbob_ret signin();
private:
	c_signinbob();// can't initialized
};


#endif

