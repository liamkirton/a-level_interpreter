//////////////////////////////////
// line.cpp
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#include "line.h"

#include <iostream.h>
#include <string.h>

line::line()
{
	c_count = 0;
	indent = 0;
}

line::~line()
{
	for (int i = 0; i < c_count; i++)
	{
		delete [] commands[i];
	}
}

bool line::parse(char *code)
{
	int t_count = 0;
	char *t_buffer = new char[(strlen(code) + 1)];
	bool in_string = false;
	int l_length = strlen(code);
	for (int i = 0; i < l_length; i++)
	{
		t_buffer[t_count] = code[i];
		if ((code[i] == ' ') && (!in_string))
		{
			if (t_count != 0)
			{
				t_buffer[t_count] = '\0';
				t_count = 0;
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
			}
			continue;
		}
		else if ((code[i] == ';') && (!in_string))
		{
			if (t_count != 0)
			{
				t_buffer[t_count] = '\0';
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
			}
			break;
		}
		else if ((code[i] == '\"') && (!in_string))
		{
			in_string = true;

		}
		else if ((code[i] == '\"') && (in_string))
		{
			in_string = false;
			t_buffer[t_count + 1] = '\0';
			t_count = 0;
			commands[c_count] = new char[1024];
			strcpy(commands[c_count], t_buffer);
			c_count++;

			continue;
		}		
		else if ((code[i] == '(') && (!in_string))
		{
			if (t_count == 0)
			{
				t_buffer[t_count + 1] = '\0';
				t_count = 0;
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
			}
			else
			{
				t_buffer[t_count] = '\0';
				t_count = 0;
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
				commands[c_count] = new char[2];
				commands[c_count][0] = '(';
				commands[c_count][1] = '\0';
				c_count++;
			}
			continue;
		}
		else if ((code[i] == ')') && (!in_string))
		{
			if (t_count == 0)
			{
				t_buffer[t_count + 1] = '\0';
				t_count = 0;
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
			}
			else
			{
				t_buffer[t_count] = '\0';
				t_count = 0;
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
				commands[c_count] = new char[2];
				commands[c_count][0] = ')';
				commands[c_count][1] = '\0';
				c_count++;
			}
			continue;
		}
		else if ((code[i] == '[') && (!in_string))
		{
			if (t_count == 0)
			{
				t_buffer[t_count + 1] = '\0';
				t_count = 0;
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
			}
			else
			{
				t_buffer[t_count] = '\0';
				t_count = 0;
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
				commands[c_count] = new char[2];
				commands[c_count][0] = '[';
				commands[c_count][1] = '\0';
				c_count++;
			}

			continue;
		}
		else if ((code[i] == ']') && (!in_string))
		{
			if (t_count == 0)
			{
				t_buffer[t_count + 1] = '\0';
				t_count = 0;
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
			}
			else
			{
				t_buffer[t_count] = '\0';
				t_count = 0;
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
				commands[c_count] = new char[2];
				commands[c_count][0] = ']';
				commands[c_count][1] = '\0';
				c_count++;
			}
			continue;
		}
		else if ((code[i] == ':') && (!in_string))
		{
			if (t_count == 0)
			{
				t_buffer[t_count + 1] = '\0';
				t_count = 0;
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
			}
			else
			{
				t_buffer[t_count] = '\0';
				t_count = 0;
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
				commands[c_count] = new char[2];
				commands[c_count][0] = ':';
				commands[c_count][1] = '\0';
				c_count++;
			}
			continue;
		}
		else if ((code[i] == ',') && (!in_string))
		{
			if (t_count == 0)
			{
				t_buffer[t_count + 1] = '\0';
				t_count = 0;
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
			}
			else
			{
				t_buffer[t_count] = '\0';
				t_count = 0;
				commands[c_count] = new char[1024];
				strcpy(commands[c_count], t_buffer);
				c_count++;
				commands[c_count] = new char[2];
				commands[c_count][0] = ',';
				commands[c_count][1] = '\0';
				c_count++;
			}
			continue;
		}
		else if (i == (l_length - 1))
		{
			t_buffer[t_count + 1] = '\0';
			t_count = 0;
			commands[c_count] = new char[1024];
			strcpy(commands[c_count], t_buffer);
			c_count++;
			continue;
		}
		t_count++;
	}

	delete [] t_buffer;

	int b_level_a = 0;
	int b_level_b = 0;
	for (i = 0; i < c_count; i++)
	{
		if ((strcmp(commands[i], "(")) == 0)
		{
			b_level_a++;
		}
		else if ((strcmp(commands[i], ")")) == 0)
		{
			b_level_a--;
		}
		else if ((strcmp(commands[i], "[")) == 0)
		{
			b_level_b++;
		}
		else if ((strcmp(commands[i], "]")) == 0)
		{
			b_level_b--;
		}
	}

	if (b_level_a != 0)
	{
		char *msg = new char[1024];
		char *tmp = new char[1024];
		strcpy(msg, "Unbalanced Brackets: ( - )\nLine: ");
		strcat(msg, itoa((line_id + 1), tmp, 10));
		MessageBox(NULL, msg, "Error", MB_OK);
		delete [] msg;
		delete [] tmp;

		return false;
	}
	if (b_level_b != 0)
	{
		char *msg = new char[1024];
		char *tmp = new char[1024];
		strcpy(msg, "Unbalanced Brackets: [ - ]\nLine: ");
		strcat(msg, itoa((line_id + 1), tmp, 10));
		MessageBox(NULL, msg, "Error", MB_OK);
		delete [] msg;
		delete [] tmp;

		return false;
	}

	return true;
}
