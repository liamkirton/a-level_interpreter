//////////////////////////////////
// output.h
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#ifndef _OUTPUT_H_
#define _OUTPUT_H_ 1

#include <windows.h>

class output  
{
public:
	output();
	virtual ~output();

	int x;
	int y;
	int width;
	int height;

	int start;
	int end;
	
	int l_display;

	COLORREF colour;
	bool update;

	void print(char *text);
	void clear();

	int t_count;
	char *t_lines[1024];
};

#endif
