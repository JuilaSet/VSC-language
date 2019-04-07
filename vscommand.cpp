#include "pch.h"
#include "vscommand.h"


#define COMMAND_GET(name) \
if (str == #name)	\
	op = Command(OPERATOR::name);\

//

Command CommandHelper::getBasicCommandOfString(std::string str) {
	Command op = Command(OPERATOR::ERROR);
	COMMAND_GET(ABORT)
	else
	COMMAND_GET(NOP)
	else
	COMMAND_GET(PUSH_POS)
	else
	COMMAND_GET(POP)
	else
	COMMAND_GET(CAST_NUMBER)
	else
	COMMAND_GET(CAST_STRING)
	else
	COMMAND_GET(CAST_BOOL)
	else
	COMMAND_GET(EQL)
	else
	COMMAND_GET(NEQL)
	else
	COMMAND_GET(CMP)
	else
	COMMAND_GET(TEST)
	else
	COMMAND_GET(JG)
	else
	COMMAND_GET(JL)
	else
	COMMAND_GET(JEG)
	else
	COMMAND_GET(JEL)
	else
	COMMAND_GET(JMP)
	else
	COMMAND_GET(JMP_TRUE)
	else
	COMMAND_GET(JMP_FALSE)
	else
	COMMAND_GET(COUNT)
	else
	COMMAND_GET(NUL)
	else
	COMMAND_GET(ECX)
	else
	COMMAND_GET(REPT)
	else
	COMMAND_GET(LOCAL_BEGIN)
	else
	COMMAND_GET(LOCAL_END)
	else
	COMMAND_GET(NEW_DRF)
	else
	COMMAND_GET(NEW_DEF)
	else
	COMMAND_GET(NEW_ASSIGN)
	else
	COMMAND_GET(STRCAT)
	else
	COMMAND_GET(ADD)
	else
	COMMAND_GET(NOT)
	else
	COMMAND_GET(EQ)
	else
	COMMAND_GET(L)
	else
	COMMAND_GET(G)
	else
	COMMAND_GET(SUB)
	else
	COMMAND_GET(SHRINK)
	else
	COMMAND_GET(ISNON)
	else
	COMMAND_GET(RET)
	else
	COMMAND_GET(CALL)
	else
	COMMAND_GET(CALL_BLK)
	else {
#if CHECK_Eval
	std::cerr << "ERROR: " << str << std::endl;
#endif
	}
	return op;
}
