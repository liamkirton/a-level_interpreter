//////////////////////////////////
// function.cpp
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#include "interpreter.h"

#include <iostream.h>
#include <windows.h>

#include <fstream.h>

function::function()
{
	m_int = NULL;
	exe_pos = 0;
	v_count = 0;
	c_stack_count = 0;
	decn_stack_count = 0;
	l_count = 0;
	b_return = false;
}

function::function(interpreter *p_int)
{
	m_int = p_int;
	exe_pos = 0;
	v_count = 0;
	c_stack_count = 0;
	decn_stack_count = 0;
	l_count = 0;
	b_return = false;
}

function::~function()
{	
	delete [] name;
	delete [] type;
	for (int i = 0; i < l_count; i++)
	{
		delete c_lines[i];
	}
	for (i = 0; i < v_count; i++)
	{
		delete vars[i];
	}
	for (i = 0; i <= decn_stack_count; i++)
	{
		delete decn_stack[i];
	}
	for (i = 0; i <= c_stack_count; i++)
	{
		delete c_stack[i];
	}
}

bool function::set_func(line *n_line)
{
	name = strdup(n_line->commands[2]);
	type = strdup(n_line->commands[1]);
	parameters = 0;

	if (n_line->c_count < 5)
	{
		error("Invalid Function Definition", name, n_line->line_id);
		return false;
	}

	for (int i = 4; i < n_line->c_count; i+=4)
	{
		if ((strcmp(n_line->commands[i], "]")) == 0)
		{
			break;
		}

		vars[v_count] = new variable();
		vars[v_count]->name = strdup(n_line->commands[i+2]);

		if ((strcmp(n_line->commands[i+1], "num")) == 0)
		{
			vars[v_count]->type = NUMBER;
		}
		else if ((strcmp(n_line->commands[i+1], "str")) == 0)
		{
			vars[v_count]->type = STRING;
		}
		else
		{
			error("Invalid Parameter Type", name, n_line->line_id);
			return false;
		}
		vars[v_count]->value = new char[1024];
		strcpy(vars[v_count]->value, "-1");
		v_count++;
	}

	parameters = v_count;

	l_count = 0;
	
	return true;
}

bool function::add_line(line *n_line)
{	
	c_lines[l_count] = n_line;
	n_line->line_id = l_count;
	l_count++;
	return true;
}

bool function::add_var(line *n_line)
{
	if ((n_line->c_count != 3) && (n_line->c_count != 5))
	{
		error("Invalid Variable Definition", name, n_line->line_id);
		return false;
	}

	for (int i = 0; i < v_count; i++)
	{
		if ((strcmp(vars[i]->name, n_line->commands[2])) == 0)
		{
			error("Duplicate Variable Definition", name, n_line->line_id);
			return false;
		}
		for (int j = 0; j < m_int->f_count; j++)
		{
			if ((strcmp(vars[i]->name, m_int->f_list[j]->name)) == 0)
			{
				error("Function/Variable Name Duplication", name, n_line->line_id);
				return false;
			}
		}
	}

	vars[v_count] = new variable();
	vars[v_count]->name = new char[(strlen(n_line->commands[2]) + 1)];
	strcpy(vars[v_count]->name, n_line->commands[2]);
	
	if ((strcmp(n_line->commands[1], "str")) == 0)
	{
		vars[v_count]->type = STRING;
	}
	else if ((strcmp(n_line->commands[1], "num")) == 0)
	{
		vars[v_count]->type = NUMBER;
	}
	else
	{
		error("Invalid Variable Type", name, n_line->line_id);
		return false;
	}

	int v_type = NUMBER;
	if (n_line->c_count == 5)
	{
		for (i = 0; i < (signed)strlen(n_line->commands[4]); i++)
		{
			if ((((int)n_line->commands[4][i] < (int)'0') || ((int)n_line->commands[4][i] > (int)'9')) && 
				(n_line->commands[4][i] != '.') && (n_line->commands[4][i] != '-'))
			{
				v_type = STRING;
				break;
			}
		}
		if (v_type != vars[v_count]->type)
		{		
			error("Variable Type Error - Attempt To Set Variable With Wrong Type", name, n_line->line_id);
			return false;
		}
	}

	vars[v_count]->value = new char[100];

	if (n_line->c_count == 5)
	{
		strcpy(vars[v_count]->value, n_line->commands[4]);
	}
	else if (n_line->c_count == 3)
	{
		vars[v_count]->value[0] = '-';
		vars[v_count]->value[1] = '0';
		vars[v_count]->value[2] = '\0';
	}

	v_count++;
	return true;
}

variable *function::get_var(char *name)
{
	for (int i = 0; i < v_count; i++)
	{
		if ((strcmp(name, vars[i]->name)) == 0)
		{
			return vars[i];
		}
	}
	return NULL;
}

void function::error(char *err, char *func, int line)
{
	if (exec_type == -1)
	{
		return;
	}

	char *msg = new char[1024];
	char *buffer = new char[100];
	strcpy(msg, "");
	strcat(msg, err);
	strcat(msg, "\n\nFunction: ");
	strcat(msg, func);
	strcat(msg, "\nLine: ");
	strcat(msg, itoa((line + 1), buffer, 10));

	p_output->print("*** ERROR OCCURRED ***");
	MessageBox(NULL, msg, "Error", MB_ICONEXCLAMATION);

	if (exec_type == 2)
	{
		SetEvent(hNext);
	}
	exec_type = -1;
}

char *function::execute()
{
	ret_val = new char[1024];
	strcpy(ret_val, "-1");
	c_stack[0] = new control_block();
	c_stack[0]->start_line = -1;
	c_stack[0]->end_line = -1;
	c_stack[0]->loop = -1;
	c_stack[0]->type = -1;

	decn_stack[0] = new decision_block();
	decn_stack[0]->found_true_statement = false;
	
	b_next = true;

	while (1)
	{
		if (exec_type == -1)
		{				
			break;
		}

		if (((strcmp(c_lines[exe_pos]->commands[0], "}")) == 0) && (c_stack[c_stack_count]->end_line == exe_pos))
		{			
			if (c_stack[c_stack_count]->loop == 0)
			{
				if (c_stack[c_stack_count]->type == ELSE_BLOCK)
				{
					delete decn_stack[decn_stack_count];
					decn_stack_count--;
				}
				else
				{					
					if ((strcmp(c_lines[exe_pos+1]->commands[0], "else")) != 0)
					{						
						delete decn_stack[decn_stack_count];
						decn_stack_count--;
					}
				}
				delete c_stack[c_stack_count];
				c_stack_count--;
			}
			else
			{
				next_line();
				EnterCriticalSection(&cSection);
				if (m_int->f_current == 0)
				{
					p_main->l_colour[exe_pos] = RGB(0,0,255);
					p_main->l_colour[c_stack[c_stack_count]->start_line] = RGB(255,0,0);
					p_main->update = true;					
				}

				p_current->l_colour[exe_pos] = RGB(0,255,0);
				p_current->l_colour[c_stack[c_stack_count]->start_line] = RGB(255,255,0);
				p_current->update = true;

				exe_pos = c_stack[c_stack_count]->start_line;
				
				if (m_int->f_current == 0)
				{
					if ((p_main->end < (exe_pos + 1)) || (p_main->start > exe_pos))
					{
						p_main->start = exe_pos - 1;
						p_main->end = (p_main->height / 15 - 2);
						if (p_main->end >= p_main->t_count)
						{
							p_main->end = p_main->t_count - 1;
						}
					}
				}

				if ((p_current->end < (exe_pos + 1)) || (p_current->start > exe_pos))
				{
					p_current->start = exe_pos - 1;
					p_current->end = (p_current->height / 15 - 2);
					if (p_current->end >= p_current->t_count)
					{
						p_current->end = p_current->t_count - 1;
					}
				}
				LeaveCriticalSection(&cSection);
				continue;
			}
		}	
		
		int t_val = -1;
		line *t_expr = expression(c_lines[exe_pos], 0, c_lines[exe_pos]->c_count, &t_val);
		
		if (t_expr == NULL)
		{
			break;
		}
		line *result = command(t_expr);
		if (result == NULL)
		{
			break;
		}
		
		delete t_expr;

		for (int i = 0; i < result->c_count; i++)
		{	
			if ((strcmp(result->commands[i], "if")) == 0)
			{
				if ((atof(result->commands[i+1]) == 1) && 
					(((i != 0) && (decn_stack[decn_stack_count]->found_true_statement == false)) || (i == 0)))
				{
					if (i == 0)
					{
						p_cline->print("Entering \'if\' Block");
					}
					else
					{
						p_cline->print("Entering \'else if\' Block");
					}

					c_stack_count++;
					c_stack[c_stack_count] = new control_block();
					c_stack[c_stack_count]->start_line = exe_pos + 1;
					c_stack[c_stack_count]->end_line = -1;
					c_stack[c_stack_count]->loop = 0;

					if (i == 0)
					{
						c_stack[c_stack_count]->type = IF_BLOCK;
						decn_stack_count++;
						decn_stack[decn_stack_count] = new decision_block();
					}
					else if (i == 1)
					{
						c_stack[c_stack_count]->type = ELSEIF_BLOCK;
					}
					decn_stack[decn_stack_count]->found_true_statement = true;

					if ((strcmp(c_lines[exe_pos + 1]->commands[0], "{")) != 0)
					{
						error("Invalid \"if\" Block Definition", name, c_lines[exe_pos + 1]->line_id);
					}

					int t_indent = 0;
					for (int j = exe_pos + 1; j < l_count; j++)
					{						
						if ((strcmp(c_lines[j]->commands[0], "{")) == 0)
						{
							t_indent++;
						}
						if ((strcmp(c_lines[j]->commands[0], "}")) == 0)
						{
							if (t_indent == 1)
							{
								c_stack[c_stack_count]->end_line = j;
								break;
							}
							else
							{
								t_indent--;
							}
						}
					}

					if (c_stack[c_stack_count]->end_line < 0)
					{
						error("Invalid \"if\" Block Definition", name, c_lines[exe_pos]->line_id);
					}
					break;
				}
				else
				{
					if (i == 0)
					{
						p_cline->print("Skipping \'if\' Block");
					}
					else
					{
						p_cline->print("Skipping \'else if\' Block");
					}

					next_line();

					if (i == 0)
					{					
						decn_stack_count++;
						decn_stack[decn_stack_count] = new decision_block();					
						decn_stack[decn_stack_count]->found_true_statement = false;
					}
					
					int t_indent = 0;
					for (int j = (exe_pos + 1); j < l_count; j++)
					{
						if ((strcmp(c_lines[j]->commands[0], "{")) == 0)
						{
							t_indent++;
						}
						if ((strcmp(c_lines[j]->commands[0], "}")) == 0)
						{
							if (t_indent == 1)
							{								
								EnterCriticalSection(&cSection);
								if (m_int->f_current == 0)
								{									
									p_main->l_colour[exe_pos] = RGB(0,0,255);
									p_main->l_colour[j] = RGB(255,0,0);
									p_main->update = true;									
								}
								p_current->l_colour[exe_pos] = RGB(0,255,0);
								p_current->l_colour[j] = RGB(255,255,0);
								p_current->update = true;
								
								exe_pos = j;
								b_next = false;

								if (m_int->f_current == 0)
								{
									if ((p_main->end < exe_pos) || (p_main->start > exe_pos))
									{
										p_main->start = exe_pos - 1;
										p_main->end = p_main->start + (p_main->height / 15 - 1);
										if (p_main->end >= p_main->t_count)
										{
											p_main->end = p_main->t_count - 1;
										}
									}
								}
								if ((p_current->end < exe_pos) || (p_current->start > exe_pos))
								{
									p_current->start = exe_pos - 1;
									p_current->end = p_current->start + (p_current->height / 15 - 1);
									if (p_current->end >= p_current->t_count)
									{
										p_current->end = p_current->t_count - 1;
									}
								}

								LeaveCriticalSection(&cSection);								
								break;
							}
							else
							{
								t_indent--;
							}
						}
					}
					break;
				}
			}
			else if ((strcmp(result->commands[i], "else")) == 0)
			{				
				if ((i+1) < result->c_count)
				{
					if ((strcmp(result->commands[i+1], "if")) == 0)
					{
						continue;
					}
				}

				if (decn_stack[decn_stack_count]->found_true_statement == false)
				{
					p_cline->print("Entering \'else\' Block");

					c_stack_count++;
					c_stack[c_stack_count] = new control_block();
					c_stack[c_stack_count]->start_line = exe_pos + 1;
					c_stack[c_stack_count]->end_line = -1;
					c_stack[c_stack_count]->loop = 0;
					c_stack[c_stack_count]->type = ELSE_BLOCK;
					if ((strcmp(c_lines[exe_pos + 1]->commands[0], "{")) != 0)
					{
						error("Invalid \"else\" Block Definition", name, c_lines[exe_pos + 1]->line_id);
						continue;
					}

					int t_indent = 0;
					for (int j = (exe_pos + 1); j < l_count; j++)
					{
						if ((strcmp(c_lines[j]->commands[0], "{")) == 0)
						{
							t_indent++;
						}
						if ((strcmp(c_lines[j]->commands[0], "}")) == 0)
						{
							if (t_indent == 1)
							{
								c_stack[c_stack_count]->end_line = j;
								break;
							}
							else
							{
								t_indent--;
							}
						}
					}
					if (c_stack[c_stack_count]->end_line < 0)
					{
						error("Invalid \"else\" Block Definition", name, c_lines[exe_pos]->line_id);
						continue;
					}
					break;
				}
				else
				{
					p_cline->print("Skipping \'else\' Block");
					next_line();

					int t_indent = 0;
					for (int j = (exe_pos + 1); j < l_count; j++)
					{
						if ((strcmp(c_lines[j]->commands[0], "{")) == 0)
						{
							t_indent++;
						}
						if ((strcmp(c_lines[j]->commands[0], "}")) == 0)
						{
							if (t_indent == 1)
							{								
								EnterCriticalSection(&cSection);
								if (m_int->f_current == 0)
								{									
									p_main->l_colour[exe_pos] = RGB(0,0,255);
									p_main->l_colour[j] = RGB(255,0,0);
									p_main->update = true;
								}
								p_current->l_colour[exe_pos] = RGB(0,255,0);
								p_current->l_colour[j] = RGB(255,255,0);
								p_current->update = true;

								if (m_int->f_current == 0)
								{
									if ((p_main->end < (exe_pos + 1)) || (p_main->start > exe_pos))
									{
										p_main->start = exe_pos - 1;
										p_main->end = p_main->start + (p_main->height / 15 - 1) - 1;
										if (p_main->end >= p_main->t_count)
										{
											p_main->end = p_main->t_count - 1;
										}
									}
								}

								if ((p_current->end < (exe_pos + 1)) || (p_current->start > exe_pos))
								{
									p_current->start = exe_pos - 1;
									p_current->end = p_current->start + (p_current->height / 15 - 1) - 1;
									if (p_current->end >= p_current->t_count)
									{
										p_current->end = p_current->t_count - 1;
									}
								}

								LeaveCriticalSection(&cSection);
								exe_pos = j;
								b_next = false;
								break;
							}
							else
							{
								t_indent--;
							}
						}
					}
					break;
				}
			}
			else if ((strcmp(result->commands[i], "while")) == 0)
			{
				if (c_stack[c_stack_count]->start_line == exe_pos)
				{
					if ((strcmp(result->commands[i+1], "1")) == 0)
					{
						;
					}
					else
					{
						delete c_stack[c_stack_count];
						c_stack_count--;
						int t_indent = 0;
						for (int j = (exe_pos + 2); j < l_count; j++)
						{
							if ((strcmp(c_lines[j]->commands[0], "{")) == 0)
							{
								t_indent++;
							}
							if ((strcmp(c_lines[j]->commands[0], "}")) == 0)
							{
								if (t_indent == 0)
								{
									EnterCriticalSection(&cSection);
									if (m_int->f_current == 0)
									{										
										p_main->l_colour[exe_pos] = RGB(0,0,255);
										p_main->l_colour[j] = RGB(255,0,0);
										p_main->update = true;
										
									}
									p_current->l_colour[exe_pos] = RGB(0,255,0);
									p_current->l_colour[j] = RGB(255,255,0);
									p_current->update = true;
									
									exe_pos = j;
									b_next = false;

									if (m_int->f_current == 0)
									{
										if ((p_main->end < (exe_pos + 1)) || (p_main->start > exe_pos))
										{
											p_main->start = exe_pos - 1;
											p_main->end = p_main->start + (p_main->height / 15 - 1) - 1;
											if (p_main->end >= p_main->t_count)
											{
												p_main->end = p_main->t_count - 1;
											}
										}
									}

									if ((p_current->end < (exe_pos + 1)) || (p_current->start > exe_pos))
									{
										p_current->start = exe_pos - 1;
										p_current->end = p_current->start + (p_current->height / 15 - 1) - 1;
										if (p_current->end >= p_current->t_count)
										{
											p_current->end = p_current->t_count - 1;
										}
									}

									LeaveCriticalSection(&cSection);

									break;									
								}
								else
								{
									t_indent--;
								}						
							}
						}
					}					
				}
				else
				{
					int t_indent = 0;
					int t_line = 0;
					for (int j = (exe_pos + 2); j < l_count; j++)
					{
						if ((strcmp(c_lines[j]->commands[0], "{")) == 0)
						{
							t_indent++;
						}
						if ((strcmp(c_lines[j]->commands[0], "}")) == 0)
						{
							if (t_indent == 0)
							{
								t_line = j;
								break;
							}
							else
							{
								t_indent--;
							}
						}
					}
					if ((strcmp(result->commands[i+1], "0")) == 0)
					{						
						EnterCriticalSection(&cSection);
						if (m_int->f_current == 0)
						{							
							p_main->l_colour[exe_pos] = RGB(0,0,255);
							p_main->l_colour[t_line] = RGB(255,0,0);
							p_main->update = true;						
						}
						p_current->l_colour[exe_pos] = RGB(0,255,0);
						p_current->l_colour[t_line] = RGB(255,255,0);
						p_current->update = true;
						exe_pos = t_line;

						if (m_int->f_current == 0)
						{
							if ((p_main->end < (exe_pos + 1)) || (p_main->start > exe_pos))
							{
								p_main->start = exe_pos - 1;
								p_main->end = p_main->start + (p_main->height / 15 - 1) - 1;
								if (p_main->end >= p_main->t_count)
								{
									p_main->end = p_main->t_count - 1;
								}
							}
						}

						if ((p_current->end < (exe_pos + 1)) || (p_current->start > exe_pos))
						{
							p_current->start = exe_pos - 1;
							p_current->end = p_current->start + (p_current->height / 15 - 1) - 1;
							if (p_current->end >= p_current->t_count)
							{
								p_current->end = p_current->t_count - 1;
							}
						}
						LeaveCriticalSection(&cSection);						
						break;
					}
					else
					{
						c_stack_count++;
						c_stack[c_stack_count] = new control_block();
						c_stack[c_stack_count]->start_line = exe_pos;
						c_stack[c_stack_count]->end_line = t_line;
						c_stack[c_stack_count]->loop = 1;
						c_stack[c_stack_count]->type = WHILE_BLOCK;
					}
				}

				delete [] result->commands[i];
				delete [] result->commands[i+1];
				for (int j = i; j < (result->c_count - 2); j++)
				{
					result->commands[j] = result->commands[j+2];
				}
				result->c_count -= 2;
				i -= 1;
			}
			else if ((strcmpi(result->commands[i], "print")) == 0)
			{
				char *output = new char[1024];
				output[0] = '\0';
				
				int t_offset = 0;
				delete [] result->commands[i];

				for (int j = (i+1); j < result->c_count; j++)
				{
					t_offset++;

					if (result->commands[j][0] == '\"')
					{
						char *t_char = new char[1024];
						for (int k = 0; k < (signed)(strlen(result->commands[j]) - 2); k++)
						{
							t_char[k] = result->commands[j][k+1];
							t_char[k+1] = '\0';
						}
						delete result->commands[j];
						result->commands[j] = t_char;
					}
					
					if ((strcmp(result->commands[j], ",")) == 0)
					{
						delete [] result->commands[j];
						continue;
					}
					else if ((strcmp(result->commands[j], "[")) == 0)
					{
						delete [] result->commands[j];
						continue;
					}
					else if ((strcmp(result->commands[j], "]")) == 0)
					{
						delete [] result->commands[j];						
						break;						
					}
					else if ((strcmp(result->commands[j], "endl")) == 0)
					{	
						delete [] result->commands[j];
						strcat(output, "\n");
					}					
					else
					{						
						strcat(output, result->commands[j]);
						delete [] result->commands[j];
					}
				}

				for (j = i; j < result->c_count; j++)
				{
					if ((j + t_offset) >= (result->c_count - 1))
					{
						break;
					}
					result->commands[j] = result->commands[j+t_offset];
				}
				result->c_count -= (t_offset+1);
				i-=1;
				
				EnterCriticalSection(&cSection);
				p_output->print(output);
				LeaveCriticalSection(&cSection);
				
				delete [] output;
			}
			else if ((strcmp(result->commands[i], "return")) == 0)
			{
				if (b_return)
				{
					error("Return Value Has Already Been Set For This Function!", name, exe_pos);					
				}
				b_return = true;
				strcpy(ret_val, result->commands[i+1]);

				delete [] result->commands[i];
				delete [] result->commands[i+1];

				for (int j = i; j < (result->c_count - 2); j++)
				{
					result->commands[j] = result->commands[j+2];
				}
				result->c_count -= 2;
				i -= 1;
			}
			else if (((strcmp(result->commands[i], "{")) != 0) && ((strcmp(result->commands[i], "}")) != 0))
			{				
				char *unknown = new char[1024];
				strcpy(unknown, "Unknown Command: ");
				strcat(unknown, result->commands[i]);
				error(unknown, name, exe_pos);
				delete [] unknown;
				break;
			}
		}
		
		delete result;
		
		if (b_next)
		{
			next_line();
		}
		else
		{
			b_next = true;
		}

		if (exe_pos < (l_count - 1))
		{
			EnterCriticalSection(&cSection);
			if (m_int->f_current == 0)
			{				
				p_main->l_colour[exe_pos] = RGB(0,0,255);
				p_main->l_colour[exe_pos+1] = RGB(255,0,0);
				p_main->update = true;				
			}
			p_current->l_colour[exe_pos] = RGB(0,255,0);
			p_current->l_colour[exe_pos+1] = RGB(255,255,0);
			p_current->update = true;

			if (m_int->f_current == 0)
			{
				if (p_main->end < (exe_pos + 1))
				{
					p_main->start += (p_main->height / 15 - 1) - 1;
					p_main->end += (p_main->height / 15 - 1) - 1;
					if (p_main->end >= p_main->t_count)
					{
						p_main->end = p_main->t_count - 1;
					}
				}
			}

			if (p_current->end < (exe_pos + 1))
			{
				p_current->start += (p_current->height / 15 - 1) - 1;
				p_current->end += (p_current->height / 15 - 1) - 1;
				if (p_current->end >= p_current->t_count)
				{
					p_current->end = p_current->t_count - 1;
				}
			}
			LeaveCriticalSection(&cSection);
			exe_pos++;
		}
		else
		{
			break;
		}
	}

	if (exec_type != -1)
	{
		if (((strcmp(type, "null")) != 0) && (b_return == false))
		{
			error("Function defined as types \'num\' or \'str\' *must* return a value!", name, -1);		
		}
	}

	if (m_int->f_current == 0)
	{
		if (exec_type == -1)
		{
			p_output->print("*** EXECUTION CANCELLED ***");
		}
		exec_type = -2;
	}
	if (b_return)
	{
		return ret_val;
	}
	else
	{
		return NULL;
	}
}

void function::next_line()
{	
	EnterCriticalSection(&cSection);
	if (exec_type == 1)
	{
		LeaveCriticalSection(&cSection);
		Sleep(2500);
	}
	else if (exec_type == 2)
	{
		LeaveCriticalSection(&cSection);
		WaitForSingleObject(hNext, INFINITE);
	}
	else
	{
		LeaveCriticalSection(&cSection);
	}
	EnterCriticalSection(&cSection);
	p_cline->clear();
	LeaveCriticalSection(&cSection);
}

line *function::expression(line *l_line, int start_command, int end_command, int *p_count)
{
	line *expr = new line();
	
	for (int i = 0; i < end_command; i++)
	{
		if (((strcmp(l_line->commands[start_command + i], "(")) == 0))
		{
			line *t_line = expression(l_line, i + 1 + start_command, end_command, &i);
			if (t_line == NULL)
			{
				return NULL;
			}
			for (int j = 0; j < t_line->c_count; j++)
			{
				expr->commands[expr->c_count] = new char[1024];
				strcpy(expr->commands[expr->c_count], t_line->commands[j]);
				expr->c_count++;
			}
		}
		else if ((strcmp(l_line->commands[start_command + i], ")")) == 0) 				
		{
			line *t_line = command(expr);
			if (t_line == NULL)
			{
				return NULL;
			}
			delete expr;
			expr = new line();
			(*expr) = (*t_line);				
			*p_count += (i+1);
			break;
		}
		else
		{
			expr->commands[expr->c_count] = new char[1024];
			strcpy(expr->commands[expr->c_count], l_line->commands[start_command + i]);
			expr->c_count++;
		}		
	}
	return expr;
}

line *function::command(line *cmd)
{	
	char *t_cmd = new char[1024];
	strcpy(t_cmd, "Command: ");
	for (int i = 0; i < cmd->c_count; i++)
	{
		strcat(t_cmd, cmd->commands[i]);
		strcat(t_cmd, " ");
	}
	p_cline->print(t_cmd);
	delete [] t_cmd;

	line *result = new line();
	for (i = 0; i < cmd->c_count; i++)
	{
		result->commands[result->c_count] = new char[1024];
		strcpy(result->commands[result->c_count], cmd->commands[i]);

		for (int j = 0; j < m_int->f_count; j++)
		{
			if ((strcmp(cmd->commands[i], m_int->f_list[j]->name)) == 0)
			{				
				line *t_cmd = new line();

				for (int k = i; k < cmd->c_count; k++)
				{
					if ((strcmp(cmd->commands[k], "[")) == 0)
					{
						continue;
					}
					else if ((strcmp(cmd->commands[k], ",")) == 0)
					{
						continue;
					}
					else if ((strcmp(cmd->commands[k], "]")) == 0)
					{
						break;
					}
					t_cmd->commands[t_cmd->c_count] = new char[1024];
					strcpy(t_cmd->commands[t_cmd->c_count], cmd->commands[k]);
					t_cmd->c_count++;
				}

				for (k = 0; k < t_cmd->c_count; k++)
				{
					for (int l = 0; l < v_count; l++)
					{
						if ((strcmp(vars[l]->name, t_cmd->commands[k])) == 0)
						{
							char *t_pr = new char[1024];
							strcpy(t_pr, "  Variable: ");
							strcat(t_pr, vars[l]->name);
							strcat(t_pr, " = ");
							strcat(t_pr, vars[l]->value);
							p_cline->print(t_pr);
							delete [] t_pr;
							
							delete [] t_cmd->commands[k];
							t_cmd->commands[k] = new char[1024];
							strcpy(t_cmd->commands[k], vars[l]->value);
						}
					}
				}

				char *t_return = f_call(t_cmd, j);

				delete result->commands[i];
				if (t_return == NULL)
				{
					result->c_count--;
				}
				else
				{
					result->commands[i] = new char[1024];				
					strcpy(result->commands[i], t_return);
				}
				
				for (k = i; k < cmd->c_count; k++)
				{					
					if ((strcmp(cmd->commands[k], "]")) == 0)
					{
						i = k;
					}
				}
	
				delete [] t_return;
				delete t_cmd;
			}
		}

		result->c_count++;
	}
	
	char *t_char_a = strdup("^");
	char *t_char_b = strdup("^");
	char *t_char_c = strdup("^");

	for (i = 0; i < 3; i++)
	{
		for (int j = 0; j < result->c_count; j++)
		{
			if (((strcmp(result->commands[j], t_char_a)) == 0) || ((strcmpi(result->commands[j], t_char_b)) == 0)
				|| ((strcmpi(result->commands[j], t_char_c)) == 0))
			{				
				
				if ((j + 1) >= result->c_count)
				{
					error("Attempt To Perform Calculation With Null Command", name, exe_pos);
					return NULL;
				}

				for (int k = 0; k < v_count; k++)
				{
					if ((strcmp(vars[k]->name, result->commands[j-1])) == 0)
					{
						char *t_pr = new char[1024];
						strcpy(t_pr, "  Variable: ");
						strcat(t_pr, vars[k]->name);
						strcat(t_pr, " = ");
						strcat(t_pr, vars[k]->value);
						p_cline->print(t_pr);
						delete [] t_pr;

						delete [] result->commands[j-1];
						result->commands[j-1] = new char[1024];
						strcpy(result->commands[j-1], vars[k]->value);
					}
					if ((strcmp(vars[k]->name, result->commands[j+1])) == 0)
					{
						char *t_pr = new char[1024];
						strcpy(t_pr, "  Variable: ");
						strcat(t_pr, vars[k]->name);
						strcat(t_pr, " = ");
						strcat(t_pr, vars[k]->value);
						p_cline->print(t_pr);
						delete [] t_pr;

						delete [] result->commands[j+1];
						result->commands[j+1] = new char[1024];
						strcpy(result->commands[j+1], vars[k]->value);
					}
				}

				char *tmp = operation(result->commands[j], result->commands[j-1], result->commands[j+1]);
				
				delete [] result->commands[j-1];
				result->commands[j-1] = new char[1024];
				strcpy(result->commands[j-1], tmp);
				
				delete [] result->commands[j];
				delete [] result->commands[j+1];
				
				for (k = j; k < (result->c_count - 2); k++)
				{
					result->commands[k] = result->commands[k+2];
				}
				
				result->commands[result->c_count - 2] = NULL;
				result->commands[result->c_count - 1] = NULL;

				result->c_count -= 2;
				j -= 1;

				delete [] tmp;
			}
		}
		if (i == 0)
		{
			t_char_a[0] = '*';
			t_char_b[0] = '/';
		}
		else
		{
			t_char_a[0] = '+';
			t_char_b[0] = '-';
		}			
	}

	delete [] t_char_a;
	delete [] t_char_b;
	delete [] t_char_c;

	for (i = 0; i < result->c_count; i++)
	{
		if ((strcmp(result->commands[i], "=")) == 0)
		{	
			if ((i + 1) >= result->c_count)
			{
				error("Attempt To Set Variable With Null Command", name, exe_pos);
				break;
			}

			for (int j = 0; j < v_count; j++)
			{
				if ((strcmp(vars[j]->name, result->commands[i+1])) == 0)
				{
					char *t_pr = new char[1024];
					strcpy(t_pr, "  Variable: ");
					strcat(t_pr, vars[j]->name);
					strcat(t_pr, " = ");
					strcat(t_pr, vars[j]->value);
					p_cline->print(t_pr);
					delete [] t_pr;

					delete [] result->commands[i+1];
					result->commands[i+1] = new char[1024];
					strcpy(result->commands[i+1], vars[j]->value);
				}
			}

			set_variable(result->commands[i-1], result->commands[i+1]);

			delete [] result->commands[i-1];
			delete [] result->commands[i];
			delete [] result->commands[i+1];
			
			for (j = (i-1); j < (result->c_count - 3); j++)
			{
				result->commands[j] = result->commands[j+3];
			}
				
			result->commands[result->c_count - 3] = NULL;
			result->commands[result->c_count - 2] = NULL;
			result->commands[result->c_count - 1] = NULL;
			
			result->c_count -= 3;
			i -= 1;
		}
		else if (((strcmp(result->commands[i], "==")) == 0) || ((strcmp(result->commands[i], ">")) == 0)
				 || ((strcmp(result->commands[i], "<")) == 0) || ((strcmp(result->commands[i], "<=")) == 0)
				 || ((strcmp(result->commands[i], ">=")) == 0) || ((strcmp(result->commands[i], "!=")) == 0))
		{
			bool cmp = comparison(result->commands[i], result->commands[i-1], result->commands[i+1]);

			char *tmp = new char[1024];
			if (cmp)
			{
				tmp[0] = '1';
			}
			else
			{
				tmp[0] = '0';
			}
			tmp[1] = '\0';

			delete [] result->commands[i-1];
			result->commands[i-1] = new char[1024];
			strcpy(result->commands[i-1], tmp);
			
			delete [] tmp;
				
			delete [] result->commands[i];
			delete [] result->commands[i+1];
				
			for (int j = i; j < (result->c_count - 2); j++)
			{
				result->commands[j] = result->commands[j+2];
			}
				
			result->commands[result->c_count - 2] = NULL;
			result->commands[result->c_count - 1] = NULL;

			result->c_count -= 2;
			i -= 1;
		}
	}

	for (i = 0; i < result->c_count; i++)
	{
		for (int j = 0; j < v_count; j++)
		{
			if ((strcmp(vars[j]->name, result->commands[i])) == 0)
			{
				char *t_pr = new char[1024];
				strcpy(t_pr, "  Variable: ");
				strcat(t_pr, vars[j]->name);
				strcat(t_pr, " = ");
				strcat(t_pr, vars[j]->value);
				p_cline->print(t_pr);
				delete [] t_pr;

				delete [] result->commands[i];
				result->commands[i] = new char[1024];
				strcpy(result->commands[i], vars[j]->value);
			}
		}
	}

	return result;
}

char *function::operation(char *o_operator, char *operand_a, char *operand_b)
{	
	char *op_out = new char[1024];
	char *result = new char[1024];
	
	strcpy(op_out, "  Operation: ");
	strcat(op_out, operand_a);
	strcat(op_out, " ");
	strcat(op_out, o_operator);
	strcat(op_out, " ");
	strcat(op_out, operand_b);
	
	result[0] = '\0';
	
	int t_a = -1;
	int t_b = -1;

	for (int i = 0; i < v_count; i++)
	{
		if ((strcmp(vars[i]->name, operand_a)) == 0)
		{
			char *t_pr = new char[1024];
			strcpy(t_pr, "  Variable: ");
			strcat(t_pr, vars[i]->name);
			strcat(t_pr, " = ");
			strcat(t_pr, vars[i]->value);
			p_cline->print(t_pr);
			delete [] t_pr;
			strcpy(operand_a, vars[i]->value);
			t_a = vars[i]->type;
		}
		if ((strcmp(vars[i]->name, operand_b)) == 0)
		{
			char *t_pr = new char[1024];
			strcpy(t_pr, "  Variable: ");
			strcat(t_pr, vars[i]->name);
			strcat(t_pr, " = ");
			strcat(t_pr, vars[i]->value);
			p_cline->print(t_pr);
			delete [] t_pr;

			strcpy(operand_b, vars[i]->value);
			t_b = vars[i]->type;
		}
	}

	if (operand_a[0] == '\"')
	{
		t_a = STRING;
		char *v_tmp = new char[1024];
		for (int i = 1; i < (signed)(strlen(operand_a)); i++)
		{				
			v_tmp[i-1] = operand_a[i];
			if (operand_a[i] == '\"')
			{
				v_tmp[i-1] = '\0';
				break;
			}
		}
		strcpy(operand_a, v_tmp);
		delete [] v_tmp;
	}
	else
	{
		t_a = NUMBER;
	}

	if (operand_b[0] == '\"')
	{
		t_b = STRING;
		char *v_tmp = new char[1024];
		for (int i = 1; i < (signed)(strlen(operand_b)); i++)
		{				
			v_tmp[i-1] = operand_b[i];
			if (operand_b[i] == '\"')
			{
				v_tmp[i-1] = '\0';
				break;
			}
		}
		strcpy(operand_b, v_tmp);
		delete [] v_tmp;
	}
	else
	{
		t_b = NUMBER;
	}

	if ((t_a != t_b) && ((t_a != -1) && (t_b != -1)))
	{
		error("Attempt To Operate On Different Variable Types", name, c_lines[exe_pos]->line_id);
		return strdup("-1");
	}

	double v_a = 0.0;
	double v_b = 0.0;
	double v_res = 0.0f;
	bool b_calc = true;

	if ((strcmp(o_operator, "*")) == 0)
	{
		if (t_a != NUMBER)
		{
			error("Attempt To Multiply Strings", name, c_lines[exe_pos]->line_id);
			return strdup("-1");
		}
		v_a = atof(operand_a);
		v_b = atof(operand_b);
		v_res = v_a * v_b;
	}
	else if ((strcmp(o_operator, "^")) == 0)
	{
		if (t_a != NUMBER)
		{
			error("Attempt To Multiply Strings", name, c_lines[exe_pos]->line_id);
			return strdup("-1");
		}
		v_a = atof(operand_a);
		v_b = atof(operand_b);
		
		
		if (v_b == 0.0)
		{
			v_res = 1.0;
		}
		else
		{
			v_res = v_a;
		}
		for (int i = 1; i < (int)v_b; i++)
		{
			v_res *= v_a;

		}
	}
	else if ((strcmp(o_operator, "/")) == 0)
	{
		if (t_a != NUMBER)
		{
			error("Attempt To Divide Strings", name, c_lines[exe_pos]->line_id);
			return strdup("-1");
		}

		v_a = atof(operand_a);
		v_b = atof(operand_b);
		v_res = v_a / v_b;
	}
	else if ((strcmp(o_operator, "+")) == 0)
	{
		if (t_a != NUMBER)
		{
			b_calc = false;
			strcpy(result, "\"");
			strcat(result, operand_a);
			strcat(result, operand_b);
			strcat(result, "\"");
		}
		else
		{
			v_a = atof(operand_a);
			v_b = atof(operand_b);
			v_res = v_a + v_b;
		}
	}
	else if ((strcmp(o_operator, "-")) == 0)
	{
		if (t_a != NUMBER)
		{
			error("Attempt To Take Away Strings", name, c_lines[exe_pos]->line_id);
			return strdup("-1");
		}
		v_a = atof(operand_a);
		v_b = atof(operand_b);
		v_res = v_a - v_b;
	}
	
	if (b_calc)
	{
		int dec = 0;
		int sign = 0;
		size_t n_digit = 15;
		
		char *res_t = fcvt(v_res, n_digit, &dec, &sign);
		int res_t_len = strlen(res_t);				
		
		int r_position = 0;

		if (sign != 0)
		{
			result[0] = '-';
			r_position++;
		}		

		if (dec <= 0)
		{
			result[r_position] = '0';
			result[r_position+1] = '.';
			r_position+=2;
			for (int i = 0; i < -dec; i++)
			{
				result[r_position] = '0';
				r_position++;
			}
			
			for (i = 0; i < res_t_len; i++)
			{
				result[r_position] = res_t[i];
				r_position++;
			}
			result[r_position] = '\0';
		}
		else
		{
			int incr = 0;
			for (int i = 0; i < res_t_len; i++)
			{
				if (i == dec)
				{
					result[r_position] = '.';
					incr++;
				}
				else
				{
					result[r_position] = res_t[i - incr];					
				}
				r_position++;
			}
			result[r_position] = '\0';
		}
	}

	strcat(op_out, " = ");
	strcat(op_out, result);
	p_cline->print(op_out);
	delete [] op_out;

	return result;
}

bool function::set_variable(char *var_name, char *n_value)
{	
	variable *t_var = get_var(var_name);
	char *outpt = new char[1024];
	strcpy(outpt, "  Set Variable: ");
	strcat(outpt, var_name);
	strcat(outpt, " = ");
	strcat(outpt, n_value);
	
	if (t_var == NULL)
	{
		strcpy(outpt, "Unknown Variable Name: ");
		strcat(outpt, var_name);
		error(outpt, name, c_lines[exe_pos]->line_id); 
		delete [] outpt;
		return false;
	}

	int v_type = NUMBER;
	for (int i = 0; i < (signed)strlen(n_value); i++)
	{
		if ((((int)n_value[i] < (int)'0') || ((int)n_value[i] > (int)'9')) && (n_value[i] != '.') && (n_value[i] != '-'))
		{
			v_type = STRING;
			break;
		}
	}

	if (v_type == STRING)
	{		
		if ((n_value[0] != '\"') || (n_value[(strlen(n_value) - 1)] != '\"'))
		{
			error("String Values Should Be Enclosed In \"s", name, c_lines[exe_pos]->line_id); 
			return false;
		}
	}

	if (v_type != t_var->type)
	{		
		error("Variable Type Error - Attempt To Set Variable With Wrong Type", name, c_lines[exe_pos]->line_id);
		return false;
	}
	
	
	p_cline->print(outpt);
	delete [] outpt;

	strcpy(t_var->value, n_value);
	return true;
}

bool function::comparison(char *oper, char *val_a, char *val_b)
{
	char *cmp_out = new char[1024];
	strcpy(cmp_out, "  Comparison: ");
	strcat(cmp_out, val_a);
	strcat(cmp_out, " ");
	strcat(cmp_out, oper);
	strcat(cmp_out, " ");
	strcat(cmp_out, val_b);
	strcat(cmp_out, " == ");
	
	int t_a = -1;
	int t_b = -1;

	for (int i = 0; i < v_count; i++)
	{
		if ((strcmp(vars[i]->name, val_a)) == 0)
		{
			strcpy(val_a, vars[i]->value);
			t_a = vars[i]->type;
		}
		if ((strcmp(vars[i]->name, val_b)) == 0)
		{
			strcpy(val_b, vars[i]->value);
			t_b = vars[i]->type;
		}
	}

	for (i = 0; i < (signed)strlen(val_a); i++)
	{
		if ((((int)val_a[i] < (int)'0') || ((int)val_a[i] > (int)'9')) && (val_a[i] != '.'))
		{
			if (t_a == NUMBER)
			{
				error("Unknown Variable Error", name, c_lines[exe_pos]->line_id);
				return false;
			}
			t_a = STRING;
			char *v_tmp = new char[1024];
			for (int j = 1; j < (signed)(strlen(val_a)); j++)
			{				
				v_tmp[j-1] = val_a[j];
				if (val_a[j] == '\"')
				{
					v_tmp[j-1] = '\0';
					break;
				}
			}
			strcpy(val_a, v_tmp);
			delete [] v_tmp;
			break;
		}
	}
	for (i = 0; i < (signed)strlen(val_b); i++)
	{
		if ((((int)val_b[i] < (int)'0') || ((int)val_b[i] > (int)'9')) && (val_b[i] != '.'))
		{
			if (t_b == NUMBER)
			{
				error("Unknown Variable Error", name, c_lines[exe_pos]->line_id);
				return false;
			}
			t_b = STRING;
			char *v_tmp = new char[1024];
			for (int j = 1; j < (signed)(strlen(val_b)); j++)
			{				
				v_tmp[j-1] = val_b[j];
				if (val_b[j] == '\"')
				{
					v_tmp[j-1] = '\0';
					break;
				}
			}
			strcpy(val_b, v_tmp);
			delete [] v_tmp;
			break;
		}
	}

	if (t_a == -1)
	{
		t_a = NUMBER;
	}
	if (t_b == -1)
	{
		t_b = NUMBER;
	}

	if (t_a != t_b)
	{
		error("Attempt To Compare Different Types", name, c_lines[exe_pos]->line_id);
		return false;
	}
	else if ((t_a == STRING) && ((strcmp(oper, "==")) != 0))
	{
		error("Invalid String Comparison: < or >", name, c_lines[exe_pos]->line_id);
		return false;
	}

	bool result = false;

	if (t_a == STRING)
	{
		if ((strcmp(val_a, val_b)) == 0)
		{
			result = true;
		}
	}
	else
	{
		double t_va = atof(val_a);
		double t_vb = atof(val_b);
		
		if ((strcmp(oper, "==")) == 0)
		{
			if (t_va == t_vb)
			{
				result = true;
			}
		}
		else if ((strcmp(oper, "<")) == 0)
		{
			if (t_va < t_vb)
			{
				result = true;
			}
		}
		else if ((strcmp(oper, ">")) == 0)
		{
			if (t_va > t_vb)
			{
				result = true;
			}
		}
		else if ((strcmp(oper, "<=")) == 0)
		{
			if (t_va <= t_vb)
			{
				result = true;
			}
		}
		else if ((strcmp(oper, ">=")) == 0)
		{
			if (t_va >= t_vb)
			{
				result = true;
			}
		}
		else if ((strcmp(oper, "!=")) == 0)
		{
			if (t_va != t_vb)
			{
				result = true;
			}
		}		
	}

	if (result)
	{
		strcat(cmp_out, "true");
	}
	else
	{
		strcat(cmp_out, "false");
	}

	p_cline->print(cmp_out);
	delete [] cmp_out;

	return result;
}

char *function::f_call(line *f_cmds, int f_id)
{	
	p_cline->print("");
	char *outpt = new char[1024];
	strcpy(outpt, "Function Call: ");
	strcat(outpt, f_cmds->commands[0]);
	strcat(outpt, " [");

	for (int i = 1; i < f_cmds->c_count; i++)
	{
		
		strcat(outpt, f_cmds->commands[i]);
		if (i < (f_cmds->c_count - 1))
		{
			strcat(outpt, ", ");
		}			
	}
	strcat(outpt, "]");
	p_cline->print(outpt);

	if (exec_type == 2)
	{
		WaitForSingleObject(hNext, INFINITE);
	}

	m_int->f_current++;
	m_int->f_stack[m_int->f_current] = new function_stack;
	m_int->f_stack[m_int->f_current]->funct = new function(m_int);
	
	function *t_func = m_int->f_stack[m_int->f_current]->funct;

	for (i = 0; i < m_int->f_list[f_id]->l_count; i++)
	{
		m_int->f_stack[m_int->f_current]->funct->c_lines[m_int->f_stack[m_int->f_current]->funct->l_count] = new line();
		m_int->f_stack[m_int->f_current]->funct->c_lines[m_int->f_stack[m_int->f_current]->funct->l_count]->indent = m_int->f_list[f_id]->c_lines[i]->indent;

		m_int->f_stack[m_int->f_current]->funct->c_lines[m_int->f_stack[m_int->f_current]->funct->l_count]->line_id = m_int->f_list[f_id]->c_lines[i]->line_id;
		for (int j = 0; j < m_int->f_list[f_id]->c_lines[i]->c_count; j++)
		{
			m_int->f_stack[m_int->f_current]->funct->c_lines[m_int->f_stack[m_int->f_current]->funct->l_count]->commands[j] = new char[1024];
			strcpy(m_int->f_stack[m_int->f_current]->funct->c_lines[m_int->f_stack[m_int->f_current]->funct->l_count]->commands[j], m_int->f_list[f_id]->c_lines[i]->commands[j]);
			
			m_int->f_stack[m_int->f_current]->funct->c_lines[i]->c_count++;
		}
		
		m_int->f_stack[m_int->f_current]->funct->l_count++;		
	}
	
	m_int->f_stack[m_int->f_current]->funct->name = new char[1024];
	m_int->f_stack[m_int->f_current]->funct->type = new char[1024];

	strcpy(m_int->f_stack[m_int->f_current]->funct->name, m_int->f_list[f_id]->name);
	strcpy(m_int->f_stack[m_int->f_current]->funct->type, m_int->f_list[f_id]->type);

	for (i = 0; i < m_int->f_list[f_id]->v_count; i++)
	{
		m_int->f_stack[m_int->f_current]->funct->vars[m_int->f_stack[m_int->f_current]->funct->v_count] = new variable();
		m_int->f_stack[m_int->f_current]->funct->vars[m_int->f_stack[m_int->f_current]->funct->v_count]->name = new char[1024];
		strcpy(m_int->f_stack[m_int->f_current]->funct->vars[m_int->f_stack[m_int->f_current]->funct->v_count]->name, m_int->f_list[f_id]->vars[i]->name);
		m_int->f_stack[m_int->f_current]->funct->vars[m_int->f_stack[m_int->f_current]->funct->v_count]->type = m_int->f_list[f_id]->vars[i]->type;
		m_int->f_stack[m_int->f_current]->funct->vars[m_int->f_stack[m_int->f_current]->funct->v_count]->value = new char[1024];
		strcpy(m_int->f_stack[m_int->f_current]->funct->vars[m_int->f_stack[m_int->f_current]->funct->v_count]->value, m_int->f_list[f_id]->vars[i]->value);		
		m_int->f_stack[m_int->f_current]->funct->v_count++;		
	}

	m_int->f_stack[m_int->f_current]->funct->parameters = m_int->f_list[f_id]->parameters;

	if ((f_cmds->c_count - 1) != m_int->f_stack[m_int->f_current]->funct->parameters)
	{
		error("Invalid Number Of Parameters In Function Call", name, c_lines[exe_pos]->line_id);
		return NULL;
	}
	
	for (i = 0; i < f_cmds->c_count - 1; i++)
	{
		strcpy(m_int->f_stack[m_int->f_current]->funct->vars[i]->value, f_cmds->commands[i+1]);
	}

	EnterCriticalSection(&cSection);
	int t_count = p_current->t_count;
	int start = p_current->start;
	int end = p_current->end;
	char *t_lines[1024];
	COLORREF l_colour[1024];

	p_current->initialised = false;
	for (i = 0; i < p_current->t_count; i++)
	{
		t_lines[i] = new char[1024];
		strcpy(t_lines[i], p_current->t_lines[i]);
		delete [] p_current->t_lines[i];
		l_colour[i] = p_current->l_colour[i];
	}
	LeaveCriticalSection(&cSection);

	EnterCriticalSection(&cSection);

	p_cfunc->print(m_int->f_stack[m_int->f_current]->funct->name);

	for (i = 0; i < m_int->f_stack[m_int->f_current]->funct->l_count; i++)
	{
		p_current->t_lines[i] = new char[1024];
		p_current->t_lines[i][0] = '\0';

		for (int j = 0; j < m_int->f_stack[m_int->f_current]->funct->c_lines[i]->indent; j++)
		{
			strcat(p_current->t_lines[i], "  ");
		}
		for (j = 0; j < m_int->f_stack[m_int->f_current]->funct->c_lines[i]->c_count; j++)
		{
			strcat(p_current->t_lines[i], m_int->f_stack[m_int->f_current]->funct->c_lines[i]->commands[j]);
			strcat(p_current->t_lines[i], " ");
		}
		p_current->l_colour[i] = RGB(0,255,0);
	}
	
	p_current->t_count = m_int->f_stack[m_int->f_current]->funct->l_count;
	p_current->start = 0;
	p_current->end = p_current->t_count - 1;
	if (p_current->t_count > (p_current->height / 15 - 1))
	{
		p_current->end = p_current->height / 15 - 2;
	}

	p_current->l_colour[0] = RGB(255,255,0);
	p_current->update = true;
	p_current->initialised = true;
	LeaveCriticalSection(&cSection);
	
	char *t_return = m_int->f_stack[m_int->f_current]->funct->execute();

	EnterCriticalSection(&cSection);
	p_current->initialised = false;
	for (i = 0; i < p_current->t_count; i++)
	{
		delete [] p_current->t_lines[i];		
	}
	
	for (i = 0; i < t_count; i++)
	{
		p_current->t_lines[i] = new char[1024];
		p_current->t_lines[i][0] = '\0';
		strcpy(p_current->t_lines[i], t_lines[i]);
		p_current->l_colour[i] = l_colour[i];
	}

	p_current->start = start;
	p_current->end = end;
	p_current->t_count = t_count;
	p_current->l_colour[0] = RGB(255,255,0);
	p_current->update = true;
	p_current->initialised = true;	

	for (i = 0; i < t_count; i++)
	{		
		delete [] t_lines[i];
	}

	delete m_int->f_stack[m_int->f_current];
	m_int->f_current--;

	p_cfunc->print(m_int->f_stack[m_int->f_current]->funct->name);
	
	LeaveCriticalSection(&cSection);

	strcat(outpt, " = ");
	if (t_return == NULL)
	{
		strcat(outpt, "null");
	}
	else
	{
		strcat(outpt, t_return);
	}
	p_cline->print(outpt);
	delete [] outpt;

	return t_return;
}