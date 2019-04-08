#include "pch.h"
#include "vsEvaluator.h"

// vsEvaluator

void vsEvaluator::load_block(block_ptr block)
{
	_push_frame();

	_flag_has_instruct = true;

	// 进入block, 从头开始执行
	this->_instruct_ptr = &block->instruct();
	this->_block_ptr = block;
	ipc = -1; // 跳转后变成0
}

void vsEvaluator::exit_block()
{
	assert(!_stk_frame.empty());
	auto& _frame = this->_stk_frame.back();
	auto _return_addr = _frame.ret.return_addr;
	auto _return_blk_id = _frame.ret.return_block_id;

	// 退出运行时栈帧
	_pop_frame();

	// 如果为空, 退出程序
	if (this->_stk_frame.empty()) {
		this->_stop = true;
	}
	else {
		// 出错, 可能因为帧没有与block的调用对应上
		assert(_return_addr != -1 && _return_blk_id != -1);

		// 返回上一个block
		block_ptr lastblock = _vm->get_block_ref(_return_blk_id);

		// 恢复现场
		this->_instruct_ptr = &lastblock->instruct();
		this->_block_ptr = lastblock;
		this->ipc = _return_addr;
	}
}

// 创建并压入栈帧
void vsEvaluator::_push_frame() throw(stack_overflow_exception) {
#if CHECK_Eval
	std::cerr << "Push frame ";
#endif
	// 查看是否栈溢出
	if (max_stack_size < _stk_frame.size())throw stack_overflow_exception(_block_ptr->id());
	_StackFrame stkframe;

	// 出错, 可能是stk_frame在错误的地方push了栈帧
	assert( this->_block_ptr == nullptr && _stk_frame.empty() || 
		this->_block_ptr != nullptr && !_stk_frame.empty());

	// 如果不是顶层的block, 就告知返回地址, 设置当前返回地址 = 当前执行的地址
	if (this->_block_ptr != nullptr) {
		stkframe.ret.return_block_id = _block_ptr->id();
		stkframe.ret.return_addr = this->ipc;
		stkframe._strong_hold = _block_ptr->strong_hold();
		// 传递信息
		assert(!_stk_frame.empty());
		auto& _temp_stkframe = current_stk_frame().temp_stkframe;
		stkframe.pass_message(_temp_stkframe); // 其中会初始化temp_stkframe
	}
	_stk_frame.emplace_back(stkframe);

#if CHECK_Eval
	std::cerr << ", frame stk size = " << _stk_frame.size() << std::endl;
#endif
}

// 弹出局部变量
void vsEvaluator::_pop_frame() {
#if CHECK_Eval
	std::cerr << "Pop frame ";
#endif
	_stk_frame.pop_back();
#if CHECK_Eval
	std::cerr << ", frame stk size = " << _stk_frame.size() << std::endl;
#endif
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
		// 如果当前是强作用域， 就不往上寻找
		else if (frame._strong_hold) {
			goto END;
		}
	}
END:
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
		// 如果当前是强作用域， 就不往上寻找
		else if (frame._strong_hold) {
			goto END;
		}
	}
END:
#if CHECK_Eval
	std::cerr << __LINE__ << "\tNEW STYLE DRF FAILED" << std::endl;
#endif
	// 不存在就返回null
	return null_data;
}

// para pass
void vsEvaluator::para_pass_data(data_ptr data) {
	auto& frame = current_stk_frame();
	// 传递给临时参数表, 会在push_frame中传递信息给新建的frame
	frame.temp_stkframe.next_paras_info.act_para_list.push_back(data);
}

// para drf
data_ptr vsEvaluator::para_get_data(size_t index) {
	/*auto rbegin = _stk_frame.rbegin();
	auto rend = _stk_frame.rend();

	for (auto it = rbegin; it != rend; ++it) {
		auto& frame = *it;
		auto& _act_para_list = frame.paras_info.act_para_list;
		// 存在就返回这个data
		if (_act_para_list.size() > index) {
			return _act_para_list[index];
		}
	}*/
	
	// 只找寻一层
	assert(!_stk_frame.empty());
	auto& frame = _stk_frame.back();
	auto& _act_para_list = frame.paras_info.act_para_list;
	// 存在就返回这个data
	if (_act_para_list.size() > index) {
		return _act_para_list[index];
	}
#if CHECK_Eval
	std::cerr << __LINE__ << "\tPARA DRF FAILED" << std::endl;
#endif
	// 不存在就返回null
	return null_data;
}

int vsEvaluator::step() {
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

int vsEvaluator::eval() {
	auto begin = 0;
	auto end = _instruct_ptr->size();
	// 一直执行， 直到stop为止
	for (ipc = begin; true; ++ipc) {
#if CHECK_Eval
		std::cerr << "IPC= " << ipc << "\t" << std::ends;
#endif
		(*_instruct_ptr)[ipc].opera(this);
#if CHECK_Eval
		std::cerr << "Data info:" << std::endl;
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

			// 将要传递给下一次的参数
			std::cerr << std::endl << "next paras info >";
			auto& act_list = frame.temp_stkframe.next_paras_info.act_para_list;
			auto size_act_list = act_list.size();
			for (int i = 0; i < size_act_list; ++i) {
				std::cout << i << '=' << act_list[i]->toEchoString() << ',' << std::ends;
			}
			
			// 遍历每一个栈帧的参数列表
			std::cerr << std::endl << "paras >";
			auto& list = frame.paras_info.act_para_list;
			auto size_list = list.size();
			for (int i = 0; i < size_list; ++i) {
				std::cout << i << '=' << list[i]->toEchoString() << ',' << std::ends;
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
