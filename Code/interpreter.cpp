//////////////////////////////////
// interpreter.cpp
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#include "interpreter.h"

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <stdlib.h>

#include "line.h"

interpreter::interpreter()
{

}

interpreter::~interpreter()
{

}

bool interpreter::load(char *file)
{
	p_output->clear();
	p_cline->clear();
	p_cfunc->clear();
	
	ifstream *i_file = new ifstream(file, ios::nocreate | ios::in);
	if (!(i_file->is_open()))
	{
		delete i_file;
		return false;
	}
	
	int in_func = 0;
	char *buffer = new char[1024];
	f_count = 0;
	int l_count = 0;
	int	indent_level = 0;
		
	while (!(i_file->eof()))
	{
    	i_file->getline(buffer, 1023, '\n');
    	
    	char *buffer_t = new char[((strlen(buffer)) + 1)];
    	int buffer_t_count = 0;
    	bool b_code_found = false;
    	for (int i = 0; i < (signed)(strlen(buffer)); i++)
    	{
    	    buffer_t[buffer_t_count] = buffer[i];
            if (((buffer[i] == ' ') && (!b_code_found)) || (buffer[i] == '\t'))
            {

            }
            else if ((buffer[i] != ' ') && (!b_code_found))
            {
            	b_code_found = true;
            	buffer_t_count++;
            }
			else if ((buffer[i] == '/') && (buffer[i-1] == '/'))
			{
				buffer_t_count--;
				break;
			}
            else
            {
            	buffer_t_count++;
            }
    	}

    	buffer_t[buffer_t_count] = '\0';
    	strcpy(buffer, buffer_t);
    	delete [] buffer_t;
    	
    	if ((strlen(buffer)) == 0)
    	{
    	    continue;
    	}
    	
		line *t_line = new line();
		t_line->line_id = l_count;

		if ((t_line->parse(buffer)) == false)
		{
			delete [] buffer;
			i_file->close();
			delete i_file;
			return false;
		}
		
		l_count++;
		t_line->indent = indent_level;

		if ((strcmp(t_line->commands[0], "func")) == 0)
		{
			in_func = 0;
			if (indent_level != 0)
			{
				MessageBox(NULL, "Unbalanced Code Block Brackets { - }", "Error", MB_OK);
				delete [] buffer;
				i_file->close();
				delete i_file;
				return false;
			}
			f_list[f_count] = new function(this);
			if (!(f_list[f_count]->set_func(t_line)))
			{
				delete [] buffer;
				i_file->close();
				delete i_file;
				return false;
			}
			else
			{
				in_func = 1;
				indent_level = 0;
			}
		}
		else if (in_func == 1)
		{
			if ((strcmp(t_line->commands[0], "{")) == 0)
			{
				indent_level++;				
			}
			else if ((strcmp(t_line->commands[0], "}")) == 0)
			{
				if (indent_level == 1)
				{
					indent_level = 0;
					t_line->indent = 0;
					in_func = 2;
				}
				else
				{
					indent_level--;
					t_line->indent--;
				}
			}
			else
			{
				if (indent_level != 1)
				{
					MessageBox(NULL, "Error In Variable Block Definition { - }", "Error", MB_OK);
					delete [] buffer;
					i_file->close();
					delete i_file;
					return false;
				}

				if ((f_list[f_count]->add_var(t_line)) == false)
				{					
					delete [] buffer;
					i_file->close();
					delete i_file;
					return false;
				}
			}
		}
		else if (in_func == 2)
		{
			if (indent_level < 0)
			{
				MessageBox(NULL, "Error In Function Block Definition { - }", "Error", MB_OK);
				delete [] buffer;
				i_file->close();
				delete i_file;
				return false;
			}

			f_list[f_count]->add_line(t_line);

			if ((strcmp(t_line->commands[0], "{")) == 0)
			{
				indent_level++;			
			}
			else if ((strcmp(t_line->commands[0], "}")) == 0)
			{
				if (indent_level == 1)
				{
					f_count++;
					in_func = 0;
					indent_level = 0;
					t_line->indent = 0;
				}
				else
				{
					indent_level--;
					t_line->indent--;
				}
			}
		}
	}

	if (indent_level != 0)
	{
		MessageBox(NULL, "Error In Function Definition - Unbalanced Brackets { - }", "Error", MB_ICONEXCLAMATION);
		exec_type = -2;
		return false;
	}

	delete [] buffer;
	delete i_file;
	return true;
}

bool interpreter::run()
{
	bool bMainFound = false;

	for (int i = 0; i < f_count; i++)
	{
		for (int j = 0; j < f_list[i]->v_count; j++)
		{
			for (int k = 0; k < f_count; k++)
			{
				if ((strcmp(f_list[i]->vars[j]->name, f_list[k]->name)) == 0)
				{
					MessageBox(NULL, "Function Name/Variable Name Duplication", "Error", MB_ICONEXCLAMATION);
					exec_type = -2;
					return false;
				}			
			}
		}
		for (j = 0; j < f_count; j++)
		{
			if (j != i)
			{
				if ((strcmp(f_list[i]->name, f_list[j]->name)) == 0)
				{
					MessageBox(NULL, "Duplicate Function Name", "Error", MB_ICONEXCLAMATION);
					exec_type = -2;
					return false;
				}
			}
		}
	}

	for (int f_id = 0; f_id < f_count; f_id++)
	{
		if ((strcmp(f_list[f_id]->name, "main")) == 0)
		{			
			f_current = 0;
			bMainFound = true;
			f_stack[f_current] = new function_stack;
			f_stack[f_current]->funct = new function(this);	
	
			for (int i = 0; i < f_list[f_id]->l_count; i++)
			{
				f_stack[f_current]->funct->c_lines[f_stack[f_current]->funct->l_count] = new line();
				f_stack[f_current]->funct->c_lines[f_stack[f_current]->funct->l_count]->indent = f_list[f_id]->c_lines[i]->indent;
				f_stack[f_current]->funct->c_lines[f_stack[f_current]->funct->l_count]->line_id = f_list[f_id]->c_lines[i]->line_id;
				for (int j = 0; j < f_list[f_id]->c_lines[i]->c_count; j++)
				{
					f_stack[f_current]->funct->c_lines[f_stack[f_current]->funct->l_count]->commands[j] = new char[1024];
					strcpy(f_stack[f_current]->funct->c_lines[f_stack[f_current]->funct->l_count]->commands[j], f_list[f_id]->c_lines[i]->commands[j]);
					f_stack[f_current]->funct->c_lines[i]->c_count++;
				}
				f_stack[f_current]->funct->l_count++;
			}
			for (i = 0; i < f_list[f_id]->v_count; i++)
			{
				f_stack[f_current]->funct->vars[f_stack[f_current]->funct->v_count] = new variable();
				f_stack[f_current]->funct->vars[f_stack[f_current]->funct->v_count]->name = new char[1024];
				strcpy(f_stack[f_current]->funct->vars[f_stack[f_current]->funct->v_count]->name, f_list[f_id]->vars[i]->name);
				f_stack[f_current]->funct->vars[f_stack[f_current]->funct->v_count]->type = f_list[f_id]->vars[i]->type;
				f_stack[f_current]->funct->vars[f_stack[f_current]->funct->v_count]->value = new char[1024];
				strcpy(f_stack[f_current]->funct->vars[f_stack[f_current]->funct->v_count]->value, f_list[f_id]->vars[i]->value);		
				f_stack[f_current]->funct->v_count++;
				f_stack[f_current]->funct->parameters = f_list[f_id]->parameters;				
			}
			break;
		}
	}

	if (bMainFound == false)
	{
		MessageBox(NULL, "No \"main\" Function Found", "Error", MB_OK);
		exec_type = -2;
		return false;
	}

	EnterCriticalSection(&cSection);

	for (i = 0; i < f_stack[f_current]->funct->l_count; i++)
	{
		p_main->t_lines[i] = new char[1024];
		p_current->t_lines[i] = new char[1024];
		p_main->t_lines[i][0] = '\0';
		p_current->t_lines[i][0] = '\0';
		
		for (int j = 0; j < f_stack[f_current]->funct->c_lines[i]->indent; j++)
		{
			strcat(p_main->t_lines[i], "  ");
			strcat(p_current->t_lines[i], "  ");
		}
		for (j = 0; j < f_stack[f_current]->funct->c_lines[i]->c_count; j++)
		{
			strcat(p_main->t_lines[i], f_stack[f_current]->funct->c_lines[i]->commands[j]);
			strcat(p_current->t_lines[i], f_stack[f_current]->funct->c_lines[i]->commands[j]);
			strcat(p_main->t_lines[i], " ");
			strcat(p_current->t_lines[i], " ");
		}
		p_main->l_colour[i] = RGB(0,0,255);
		p_current->l_colour[i] = RGB(0,255,0);
	}

	f_stack[f_current]->funct->name = new char[1024];
	f_stack[f_current]->funct->type = new char[1024];
	strcpy(f_stack[f_current]->funct->name, "main");
	strcpy(f_stack[f_current]->funct->type, "null");

	p_cfunc->print("main");
	p_main->t_count = f_stack[f_current]->funct->l_count;
	p_current->t_count = f_stack[f_current]->funct->l_count;

	p_main->l_colour[0] = RGB(255,0,0);
	p_current->l_colour[0] = RGB(255,255,0);
	p_main->update = true;
	p_current->update = true;
	p_main->initialised = true;
	p_current->initialised = true;

	p_main->start = 0;
	p_current->start = 0;
	p_main->end = p_main->height / 15 - 2;
	p_current->end = p_current->height / 15 - 2;

	if (p_main->end >= f_stack[f_current]->funct->l_count)
	{
		p_main->end = f_stack[f_current]->funct->l_count - 1;
	}
	if (p_current->end >= f_stack[f_current]->funct->l_count)
	{
		p_current->end = f_stack[f_current]->funct->l_count - 1;
	}
	
	LeaveCriticalSection(&cSection);

	WaitForSingleObject(hStartStop, INFINITE);
	char *t_return = f_stack[f_current]->funct->execute();
	
	delete f_stack[f_current];
	delete [] t_return;

	return true;
}
