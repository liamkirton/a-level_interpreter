//////////////////////////////////
// variable.cpp
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#include "variable.h"

variable::variable()
{

}

variable::~variable()
{
	delete [] name;
	delete [] value;
}
