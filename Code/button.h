//////////////////////////////////
// button.h
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#ifndef _BUTTON_H_
#define _BUTTON_H_ 1

#include <windows.h>

class button  
{
public:
	button();
	virtual ~button();

	int x;
	int y;
	int width;
	int height;

	char *txt;

	bool b_active;
	bool b_highlighted;
	bool b_pressed;

	bool update;

	COLORREF colour;
	COLORREF h_colour;
	COLORREF p_colour;
};

#endif