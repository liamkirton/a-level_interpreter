//////////////////////////////////
// code_output.h
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#ifndef _CODE_OUTPUT_H_
#define _CODE_OUTPUT_H_ 1

#include <windows.h>

class code_output  
{
public:
	code_output();
	virtual ~code_output();

	int x;
	int y;
	int width;
	int height;
	
	int start;
	int end;

	bool update;
	bool initialised;

	void print(char *text);
	void clear();

	int t_count;
	char *t_lines[1024];
	COLORREF l_colour[1024];
};

#endif
