//////////////////////////////////
// function.h
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#ifndef _FUNCTION_H_
#define _FUNCTION_H_ 1

#include "interpreter.h"

class function  
{
public:
	function();
	function(interpreter *p_int);
	virtual ~function();
	bool set_func(line *n_line);
	bool add_line(line *n_line);
	bool add_var(line *n_line);
	variable *get_var(char *name);

	line *expression(line *l_line, int start_command, int end_command, int *p_count);
	line *command(line *cmd);
	char *operation(char *o_operator, char *operand_a, char *operand_b);
	bool set_variable(char *var_name, char *n_value);
	bool comparison(char *oper, char *val_a, char *val_b);

	void error(char *err, char *func, int line);

	void next_line();

	char *f_call(line *f_cmds, int f_id);

	char *execute();

	char *name;
	char *type;
	char *ret_val;

	int l_count;
	line *c_lines[1024];
	int v_count;
	variable *vars[1024];
	
	int parameters;

	int c_stack_count;
	control_block *c_stack[1024];
	
	decision_block *decn_stack[1024];
	int decn_stack_count;

	int exe_pos;

	interpreter *m_int;

	bool b_next;
	bool b_return;
};

#endif
