//////////////////////////////////
// decision_block.h
//////////////////////////////////
// Interpreter - ©Liam Kirton 2001
//////////////////////////////////

#ifndef _DECISION_BLOCK_H_
#define _DECISION_BLOCK_H_ 1

class decision_block  
{
public:
	decision_block();
	virtual ~decision_block();

	bool found_true_statement;

};

#endif