//////////////////////////////////
// line.h
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#ifndef _LINE_H_
#define _LINE_H_ 1

#include "interpreter.h"

class line  
{
public:
	line();
	virtual ~line();
	bool parse(char *code);
	
	int line_id;

	int indent;

	int c_count;
	char *commands[1024];
};

#endif
