//////////////////////////////////
// text_label.h
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#ifndef _TEXT_LABEL_
#define _TEXT_LABEL_ 1

#include <windows.h>

class text_label  
{
public:
	text_label();
	virtual ~text_label();
	
	char *text;
	COLORREF colour;

	int x;
	int y;

	bool update;
};

#endif