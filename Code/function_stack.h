//////////////////////////////////
// function_stack.h
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#ifndef _FUNCTION_STACK_
#define _FUNCTION_STACK_ 1

#include "interpreter.h"

class function_stack  
{
public:
	function_stack();
	virtual ~function_stack();

	function *funct;
	int keep_exe_pos;
};

#endif
