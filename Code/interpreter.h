//////////////////////////////////
// interpreter.h
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_ 1

class interpreter;
class function;
class function_stack;
class line;
class variable;
class control_block;
class decision_block;

#include "main.h"
#include "function.h"
#include "function_stack.h"
#include "line.h"
#include "variable.h"
#include "control_block.h"
#include "decision_block.h"

class interpreter  
{
public:
	interpreter();
	virtual ~interpreter();
	bool load (char *file);
	bool run();

	int f_count;
	function *f_list[1024];
	int f_current;
	function_stack *f_stack[1024];
};

#endif
