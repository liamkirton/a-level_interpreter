//////////////////////////////////
// main.h
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////
#ifndef _MAIN_H_
#define _MAIN_H_ 1

#include <windows.h>
#include "output.h"
#include "code_output.h"
#include "button.h"
#include "text_label.h"

extern CRITICAL_SECTION cSection;

extern output *p_output;
extern output *p_cfunc;
extern output *p_cline;
extern code_output *p_main;
extern code_output *p_current;

extern HANDLE hStartStop;
extern HANDLE hNext;

extern int exec_type;


#endif