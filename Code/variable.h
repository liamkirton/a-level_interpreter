//////////////////////////////////
// variable.h
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#ifndef _VARIABLE_H_
#define _VARIABLE_H_ 1

#define NUMBER 0
#define STRING 1

class variable  
{
public:
	variable();
	virtual ~variable();

	char *name;
	int type;
	char *value;
};

#endif
