//////////////////////////////////
// output.cpp
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#include "output.h"

#include <string.h>

output::output()
{
	t_count = 0;
	x = 0;
	y = 0;
	width = 0;
	height = 0;
	update = true;
	l_display = 0;
	start = 0;
	end = 0;
}

output::~output()
{

}

void output::print(char *text)
{
	update = true;

	t_lines[t_count] = new char[1024];
	strcpy(t_lines[t_count], text);
	t_count++;
	end = t_count;
	start = end - l_display;

	if (start < 0)
	{
		start = 0;
	}
	else
	{
		start = start;	
	}

	if (t_count == 1024)
	{			
   		if (l_display > 1024)
		{
			l_display = 1024;
		}

		int ctr = 0;
		for (int i = 0; i < t_count; i++)
		{
			if (i < (t_count - l_display))
			{
				delete [] t_lines[i];
			}
			else
			{
				t_lines[ctr] = t_lines[i];
				t_lines[i] = NULL;
			}
		}
		t_count = l_display;
		end = t_count;
		start = end - l_display;
	}
}

void output::clear()
{
	for (int i = 0; i < t_count; i++)
	{
		delete [] t_lines[i];
	}
	t_count = 0;
	start = 0;
	end = 0;
	update = true;
}