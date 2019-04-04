#include "pch.h"
#include "vsEvaluator.h"

// vsEvaluator

void vsEvaluator::setInstructPtr(vec_command_t* ins) {
	_flag_has_instruct = true;
	this->_instruct_ptr = ins;
	init();
}

// 创建并压入栈帧
void vsEvaluator::push_frame() {
	StackFrame stkframe;
	_stk_frame.emplace_back(stkframe);
}

// 弹出局部变量
void vsEvaluator::pop_frame() {
	_stk_frame.pop_back();
}

// new def
void vsEvaluator::new_regist_identity(size_t index, data_ptr d) {
	// 从当前栈开始
	assert(!_stk_frame.empty());
	auto& frame = current_stk_frame();
	auto& var_table = frame.local_var_table;

	// 检查是否存在这个index
	if (var_table.size() > index) {
		var_table[index] = data_ptr(d);
	}
	else {
		assert(var_table.size() == index);
		var_table.emplace_back(d);
	}
}

// new assign
bool vsEvaluator::new_set_data(size_t index, data_ptr d) {
	// 从当前栈开始一一搜索
	assert(!_stk_frame.empty());
	auto rbegin = _stk_frame.rbegin();
	auto rend = _stk_frame.rend();
	for (auto it = rbegin; it != rend; ++it) {
		auto& frame = *it;
		auto& var_table = frame.local_var_table;
		// 找到后对其赋值， 返回true并退出
		if (var_table.size() > index) {
			var_table[index] = data_ptr(d);
			return true;
		}
	}
#if CHECK_Eval
	std::cerr << __LINE__ << "\tNEW STYLE ASSIGN FAILED" << std::endl;
#endif
	// 如果失败返回false
	return false;
}

// new drf
data_ptr vsEvaluator::new_get_data(size_t index) {
	// 从当前栈开始一一搜索
	assert(!_stk_frame.empty());
	auto rbegin = _stk_frame.rbegin();
	auto rend = _stk_frame.rend();
	for (auto it = rbegin; it != rend; ++it) {
		auto& frame = *it;
		auto& var_table = frame.local_var_table;
		// 存在就返回这个data
		if (var_table.size() > index) {
			return var_table[index];
		}
	}
#if CHECK_Eval
	std::cerr << __LINE__ << "\tNEW STYLE DRF FAILED" << std::endl;
#endif
	// 不存在就返回null
	return null_data;
}

int vsEvaluator::step(vsVirtualMachine* vm) {
	if (ipc == _instruct_ptr->size())return 0;
	else {
		assert(ipc < _instruct_ptr->size());
#if CHECK_Eval
		std::cerr << "IPC = " << ipc << "\t" << std::ends;
#endif
		(*_instruct_ptr)[ipc].opera(this);
		++ipc;
#if CHECK_Eval
		//  遍历所有的frame
		for (auto& frame : _stk_frame) {
			std::cerr << "[";
			// 遍历全部栈帧的操作数栈
			std::cerr << "data >" << std::ends;
			auto& stk = frame.stk;
			for (auto d : stk) {
				std::cerr << d->toTypeName() << ": " << d->toEchoString() << ',' << std::ends;
			}

			// 遍历每一个栈帧的局部变量表
			std::cerr << std::endl << "identifier >";
			auto& table = frame.local_var_table;
			auto size = table.size();
			for (int i = 0; i < size; ++i) {
				std::cout << i << '=' << table[i]->toEchoString() << ',' << std::ends;
			}
			std::cerr << "] " << std::endl;
		}
		std::cerr << std::endl << std::endl;
#endif
		return 1;
	}
}

int vsEvaluator::eval(vsVirtualMachine* vm) {
	auto begin = 0;
	auto end = _instruct_ptr->size();
	init();
	for (ipc = begin; ipc != end; ++ipc) {
#if CHECK_Eval
		std::cerr << "IPC= " << ipc << "\t" << std::ends;
#endif
		(*_instruct_ptr)[ipc].opera(this);
#if CHECK_Eval
		//  遍历所有的frame
		for (auto& frame: _stk_frame) {
			std::cerr << "[";
			// 遍历全部栈帧的操作数栈
			std::cerr << "data >" << std::ends;
			auto& stk = frame.stk;
			for (auto d : stk) {
				std::cerr << d->toTypeName() << ": " << d->toEchoString() << ',' << std::ends;
			}

			// 遍历每一个栈帧的局部变量表
			std::cerr << std::endl << "identifier >";
			auto& table = frame.local_var_table;
			auto size = table.size();
			for (int i = 0; i < size; ++i) {
				std::cout << i << '=' << table[i]->toEchoString() << ',' << std::ends;
			}
			std::cerr << "] " << std::endl;
		}
		std::cerr << std::endl << std::endl;
#endif
		if (_stop)break;
	}
#if CHECK_Eval
	std::cerr << "VM run finished!" << std::endl;
#endif
	return _stop_val;
}

vsEvaluator::~vsEvaluator() { }

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
	else {
#if CHECK_Eval
	std::cerr << "ERROR: " << str << std::endl;
#endif
	}
	return op;
}
