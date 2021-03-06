﻿#include "pch.h"
#include "VirtualMachine.h"

// VirtualMachine

void VirtualMachine::setInstruct(vec_command_t ins) {
	delete_instruct();	// 删除之前的指令
	_flag_has_instruct = true;
	this->instruct = ins;
	init();
}

// 创建并压入局部变量表
void VirtualMachine::push_local_list() {
	// 变量列表
	data_list_t data_list;
	_var_list.emplace_back(data_list);

	// 新局部变量表
	new_data_list_t new_data_list;
	_new_var_list.emplace_back(new_data_list);
}

// 弹出局部变量表(销毁局部变量)
void VirtualMachine::pop_local_list() {
	_var_list.pop_back();
	_new_var_list.pop_back();
}

// def
void VirtualMachine::regist_identity(std::string id, data_ptr d) {
	assert(!_var_list.empty());
	data_list_t& data_list = _var_list.back();
	auto it = data_list.find(id);
	if (it != data_list.end()) {
		// 如果已经存在, 就覆盖之前的数据
		it->second = d;
	}
	else {
		data_list.emplace(id, d);
	}
}

// new def
void VirtualMachine::new_regist_identity(int index, data_ptr d) {
	assert(!_new_var_list.empty());
	assert(index != -1);
	auto& data_list = _new_var_list.back();
	// 检查是否存在这个index
	bool has = data_list.size() > index;
	if(has){
		data_list[index] = data_ptr(d);
	}
	else {
		assert(data_list.size() == index);
		data_list.emplace_back(d);
	}
}

// assign
bool VirtualMachine::set_data(std::string id, data_ptr d) {
	assert(!_var_list.empty());
	auto rbegin = _var_list.rbegin();
	auto rend = _var_list.rend();
	for (auto rit = rbegin; rit != rend; ++rit) {
		if (rit->count(id) > 0) {
			rit->operator[](id) = data_ptr(d);
			return true;
		}
	}
#if CHECK_Eval
	std::cerr << __LINE__ << "\tASSIGN FAILED" << std::endl;
#endif
	return false;
}

// new assign
bool VirtualMachine::new_set_data(int index, data_ptr d) {
	assert(!_new_var_list.empty());
	assert(index != -1);
	auto rbegin = _new_var_list.rbegin();
	auto rend = _new_var_list.rend();
	for (auto rit = rbegin; rit != rend; ++rit) {
		auto& data_list = *rit;
		// 存在就赋值并退出
		bool has = data_list.size() > index;
		if (has) {
			data_list[index] = data_ptr(d);
			return true;
		}
	}
#if CHECK_Eval
	std::cerr << __LINE__ << "\tNEW STYLE ASSIGN FAILED" << std::endl;
#endif
	return false;
}

// drf
data_ptr VirtualMachine::get_data(std::string id) {
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
	return null_data;
}

// new drf
data_ptr VirtualMachine::new_get_data(int index) {
	assert(!_new_var_list.empty());
	assert(index != -1);
	auto rbegin = _new_var_list.rbegin();
	auto rend = _new_var_list.rend();
	for (auto rit = rbegin; rit != rend; ++rit) {
		auto& data_list = *rit;
		// 存在就返回这个data
		bool has = data_list.size() > index;
		if (has) {
			return data_list[index];
		}
	}
#if CHECK_Eval
	std::cerr << __LINE__ << "\tNEW STYLE DRF FAILED" << std::endl;
#endif
	// 不存在就返回null
	return null_data;
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
			std::cerr << "[" << d->toTypeName() << ": " << d->toEchoString() << "]," << std::ends;
		}
		std::cerr << std::endl << "identifier >" << std::ends;
		for (auto& set : _var_list) {
			for (auto& pair : set) {
				std::cerr << "[" << pair.first << "= " << pair.second->toEchoString() << "]," << std::ends;
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
			std::cerr << "[" << d->toTypeName() << ": " << d->toEchoString() << "]," << std::ends;
		}
		std::cerr << std::endl << "identifier >";
		for (auto& set : _var_list) {
			for (auto& pair : set) {
				std::cerr << "[" << pair.first << "= " << pair.second->toEchoString() << "]," << std::ends;
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
	return _stop_val;
}

void VirtualMachine::delete_instruct() {
	if (_flag_has_instruct) {
		instruct.clear();
		_flag_has_instruct = false;
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
	else
	COMMAND_GET(SHRINK)
	else
	COMMAND_GET(ISNON)
	else
	COMMAND_GET(RET)
	else
	COMMAND_GET(CALL)
	else {
#if CHECK_Eval
	std::cerr << "ERROR: " << str << std::endl;
#endif
	}
	return op;
}
