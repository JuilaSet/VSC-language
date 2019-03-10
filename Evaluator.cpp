#include "pch.h"
#include "Evaluator.h"

// VirtualMachine

void VirtualMachine::setInstruct(vec_ins_t ins) {
	delete_instruct();	// 删除之前的指令
	_flag_delete = true;
	this->instruct = ins;
	init();
}

Data VirtualMachine::get_data(std::string id) {
	auto rbegin = _var_list.rbegin();
	auto rend = _var_list.rend();
	for (auto rit = rbegin; rit != rend; ++rit) {
		if (rit->count(id) > 0) {
			return rit->find(id)->second;
		}
	}
#if CHECK_Eval
	std::cerr << __LINE__ << "\tDRF FAILED" << std::endl;
#endif
	return Data();
}

// 创建并压入局部变量表
void VirtualMachine::push_local_list() {
	data_list_t data_list; // 变量列表
	_var_list.emplace_back(data_list);
}

// 弹出局部变量表(销毁局部变量)
void VirtualMachine::pop_local_list() {
	_var_list.pop_back();
}

// assign
void VirtualMachine::set_data(std::string id, Data d) {
	assert(!_var_list.empty());
	data_list_t& data_list = _var_list.back();
	auto rbegin = _var_list.rbegin();
	auto rend = _var_list.rend();
	for (auto rit = rbegin; rit != rend; ++rit) {
		if (rit->count(id) > 0) {
			rit->operator[](id) = d;
			return;
		}
	}
#if CHECK_Eval
	std::cerr << __LINE__ << "\tASSIGN FAILED" << std::endl;
#endif
}

// def
void VirtualMachine::regist_identity(std::string id, Data d) {
	assert(!_var_list.empty());
	data_list_t& data_list = _var_list.back();
	data_list.emplace(id, d);
}

void VirtualMachine::clear_data_list() {
	_var_list.clear();
}

void VirtualMachine::init() {
	_stop = false;
	ipc = 0;
	stk.clear();
}

int VirtualMachine::step() {
	if (ipc == instruct.size())return 0;
	else {
		assert(ipc < instruct.size());
#if CHECK_Eval
		std::cerr << "IPC = " << ipc << "\t" << std::ends;
#endif
		instruct[ipc].opera(this);
		++ipc;
#if CHECK_Eval
		std::cerr << "data >" << std::ends;
		for (auto d : stk) {
			std::cerr << "[" << d.toTypeName() << ": " << d.toEchoString() << "]," << std::ends;
		}
		std::cerr << std::endl << "identifier >" << std::ends;
		for (auto& set : _var_list) {
			for (auto& pair : set) {
				std::cerr << "[" << pair.first << "= " << pair.second.toEchoString() << "]," << std::ends;
			}
		}
		std::cerr << std::endl << std::endl;
#endif
		return 1;
	}
}

int VirtualMachine::run() {
	auto begin = 0;
	auto end = instruct.size();
	init();
	for (ipc = begin; ipc != end; ++ipc) {
#if CHECK_Eval
		std::cerr << "IPC= " << ipc << "\t" << std::ends;
#endif
		instruct[ipc].opera(this);
#if CHECK_Eval
		std::cerr << "data >" << std::ends;
		for (auto d : stk) {
			std::cerr << "[" << d.toTypeName() << ": " << d.toEchoString() << "]," << std::ends;
		}
		std::cerr << std::endl << "identifier >";
		for (auto& set : _var_list) {
			for (auto& pair : set) {
				std::cerr << "[" << pair.first << "= " << pair.second.toEchoString() << "]," << std::ends;
			}
		}
		std::cerr << std::endl << std::endl;
#endif
		if (_stop)break;
	}
	stk.clear();
#if CHECK_Eval
	std::cerr << "VM run finished!" << std::endl;
	std::cerr << "stack size = " << stk.size() << std::endl;
#endif
	return 0;
}

void VirtualMachine::delete_instruct() {
	if (_flag_delete) {
		instruct.clear();
		_flag_delete = false;
	}
}

VirtualMachine::~VirtualMachine() {
	delete_instruct();
}


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
	COMMAND_GET(REPT)
	else
	COMMAND_GET(LOCAL_BEGIN)
	else
	COMMAND_GET(LOCAL_END)
	else
	COMMAND_GET(DRF)
	else
	COMMAND_GET(DEF)
	else
	COMMAND_GET(ASSIGN)
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
	else {
#if CHECK_Eval
	std::cerr << "ERROR: " << str << std::endl;
#endif
	}
	return op;
}

Evaluator::Evaluator()
{
}

Evaluator::~Evaluator()
{
}
