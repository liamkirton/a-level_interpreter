//////////////////////////////////
// code_output.cpp
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#include "code_output.h"

code_output::code_output()
{
	t_count = 0;
	x = 0;
	y = 0;
	width = 0;
	height = 0;
	update = true;
	start = 0;
	end = 0;
	initialised = false;
}

code_output::~code_output()
{

}