//////////////////////////////////
// control_block.h
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#ifndef _CONTROL_BLOCK_H_
#define _CONTROL_BLOCK_H_ 1

#define IF_BLOCK 1
#define ELSEIF_BLOCK 2
#define ELSE_BLOCK 3

#define WHILE_BLOCK 4

class control_block  
{
public:
	control_block();
	virtual ~control_block();

	int start_line;
	int end_line;
	int loop;
	int type;
};

#endif