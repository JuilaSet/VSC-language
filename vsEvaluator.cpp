#include "pch.h"
#include "vsEvaluator.h"

// vsEvaluator

// 在此之前要使用push_next_temp_paras_info(), 防止exit_block的时候出错
void vsEvaluator::load_block(block_ptr block, int paras_count)
{
	// 设置并传递栈帧 []][
	_push_and_setting_frame(this->_block_ptr, block, paras_count);
	
	// 这里之后是内层
	_flag_has_instruct = true;

	// 进入block, 从头开始执行
	this->_instruct_ptr = &block->instruct();
	this->_block_ptr = block;
	ipc = -1; // 跳转后变成0

}

void vsEvaluator::exit_block()
{
	assert(!_stk_frame.empty());
	auto _frame = this->_stk_frame.back();
	auto _return_addr = _frame->ret.return_addr;
	auto _return_blk_id = _frame->ret.return_block_id;

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

		// 从call_blk中返回
		assert(!_stk_frame.empty());
		_frame = this->_stk_frame.back();
		assert(!_frame->temp_stkframe.next_paras_info.empty());
		_frame->pop_next_temp_paras_info();
	}
}

// 创建并压入栈帧, 设置参数个数和帧情况
void vsEvaluator::_push_and_setting_frame(block_ptr retblock, block_ptr curblock, int paras_count) throw(stack_overflow_exception) {
#if CHECK_Eval
	std::cerr << "Push frame ";
#endif
	// 查看是否栈溢出
	if (max_stack_size < _stk_frame.size())throw stack_overflow_exception(retblock->id());
	RunTimeStackFrame_ptr stkframe(new _StackFrame);

	// 如果不是顶层的block, 就告知返回地址, 设置当前返回地址 = 当前执行的地址
	if (!_stk_frame.empty()) {
		assert(retblock != nullptr);
		// 这里是外层的block
		stkframe->ret.return_block_id = retblock->id();
		stkframe->ret.return_addr = this->ipc;
#if	CHECK_Compiler_Field_NEW_BLK
		std::cerr << __LINE__ << " Frame_Field: " << (stkframe->_strong_hold ? "T" : "F") << std::endl;
#endif
		// 设置当前的block
		stkframe->_strong_hold = curblock->strong_hold();

		// 传递信息
		auto& _temp_stkframe = current_stk_frame()->temp_stkframe;
		stkframe->pass_message(_temp_stkframe, paras_count); // 其中会初始化temp_stkframe
	}
	// push栈帧
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

// 根据index获取data对象
data_ptr vsEvaluator::_find_data(int index) {
	data_ptr ret;
	bool flag_found = false;

	// 查看是否是局部变量
	auto frame = current_stk_frame();
	auto& var_table = frame->local_var_table;
	if (var_table.size() > index) {
		ret = var_table[index];
		flag_found = true;
	}
	
	// 查看是否是该函数的参数
//	if (!flag_found) {
//		auto data_p = _find_para(index);
//		if (data_p != nullptr) {
//			ret = data_p;
//			flag_found = true;
//		}
//	}

	// 查看作用域链
	if (!flag_found) {
		auto runtime_context_ptr = frame->get_current_function_context();
		while (runtime_context_ptr != nullptr) {
			// 查看外部环境的局部变量表
			auto& table = runtime_context_ptr->local_var_table;
			auto& _act_para_list = frame->paras_info.act_para_list;
			if (table.size() > index) {
				ret = table[index];
				flag_found = true;
				break;
			}
			// 查看是否是闭包的外部实参
//			else if (_act_para_list.size() > index) {
//				ret = _act_para_list[index];
//				flag_found = true;
//				break;
//			}
			// 找到强作用域为止
			else if (runtime_context_ptr->_strong_hold) {
#if CHECK_Eval
				std::cerr << __LINE__ << "\tNEW STYLE DRF FAILED" << std::endl;
#endif
				break;
			}
			// 继续
			runtime_context_ptr = runtime_context_ptr->get_current_function_context();
		}
	}

	// 查看是否是外部作用域变量
	if (!flag_found) {
		auto rbegin = _stk_frame.rbegin();
		auto rend = _stk_frame.rend();
		for (auto it = rbegin; it != rend; ++it) {
			auto& frame = *it;
			auto& var_table = frame->local_var_table;
			// 存在就返回这个data
			if (var_table.size() > index) {
				ret = var_table[index];
				flag_found = true;
				break;
			}
			// 如果当前是强作用域， 就不往上寻找
			else if (frame->_strong_hold) {
#if CHECK_Eval
				std::cerr << __LINE__ << "\tNEW STYLE DRF FAILED" << std::endl;
#endif
				break;
		}
	}
	}

	// 不存在就返回NULL_DATA
	return (flag_found ? ret : nullptr);
}

// 根据index获取para实参对象
data_ptr vsEvaluator::_find_para(int index) {
	// 只找寻一层
	auto& frame = _stk_frame.back();
	auto& _act_para_list = frame->paras_info.act_para_list;
	// 存在就返回这个data
	if (_act_para_list.size() > index) {
		return _act_para_list[index];
	}
	else {
		return nullptr;
	}
}

// new def
void vsEvaluator::new_regist_identity(size_t index, data_ptr d) {
	// 从当前栈开始
	assert(!_stk_frame.empty());
	auto frame = current_stk_frame();
	auto& var_table = frame->local_var_table;

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
bool vsEvaluator::new_set_data(size_t index, data_ptr d, bool isCopyMode) {
	// 从当前栈开始一一搜索
	assert(!_stk_frame.empty());
	auto data = _find_data(index);
	if (data == nullptr) {
#if CHECK_Eval
		std::cerr << __LINE__ << "\tNEW STYLE ASSIGN FAILED" << std::endl;
#endif
		return false;
	}
	// 是否是copy
	if (isCopyMode)
		data->cp(d);
	else
		data = data_ptr(d);
	return true;
}

// new drf
data_ptr vsEvaluator::new_get_data(size_t index) {
	assert(!_stk_frame.empty());
	auto data = _find_data(index);
	if (data != nullptr) {
		return data;
	}
	else {
		return null_data;
	}
}

// para pass
void vsEvaluator::para_pass_data(data_ptr data) {
	auto frame = current_stk_frame();
	// 传递给临时参数表, 会在push_frame中传递信息给新建的frame
	assert(!frame->temp_stkframe.next_paras_info.empty());
	frame->temp_stkframe.backParasInfo().act_para_list.push_back(data);
}

// para assign
bool vsEvaluator::para_assign_data(size_t index, data_ptr data, bool isCopyMode) {
	// 只找寻一层
	assert(!_stk_frame.empty());
	auto p_data = _find_para(index);
	if (p_data == nullptr) {
#if CHECK_Eval
		std::cerr << __LINE__ << "\tPARA ASSIGN FAILED" << std::endl;
#endif
		// 不存在
		return false;
	}
	// 是否是copy
	if (isCopyMode)
		p_data->cp(data);
	else
		p_data = data;
	return true;
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
	
	assert(!_stk_frame.empty());
	auto p_data = _find_para(index);
	if (p_data == nullptr) {
#if CHECK_Eval
		std::cerr << __LINE__ << "\tPARA DRF FAILED" << std::endl;
#endif
		// 不存在就返回null
		return null_data;
	}
	return p_data;
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
		for (auto frame : _stk_frame) {
			std::cerr << "[";
			// 遍历全部栈帧的操作数栈
			std::cerr << "data >" << std::ends;
			auto& stk = frame->stk;
			for (auto d : stk) {
				std::cerr << d->getTypeName() << ": " << d->toEchoString() << ',' << std::ends;
			}

			// 遍历每一个栈帧的局部变量表
			std::cerr << std::endl << "identifier >";
			auto& table = frame->local_var_table;
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
			auto& stk = frame->stk;
			for (auto d : stk) {
				std::cerr << d->getTypeName() << ": " << d->toEchoString() << ',' << std::ends;
			}

			// 遍历每一个栈帧的局部变量表
			std::cerr << std::endl << "identifier >";
			auto& table = frame->local_var_table;
			auto size = table.size();
			for (int i = 0; i < size; ++i) {
				std::cout << i << '=' << table[i]->toEchoString() << ',' << std::ends;
			}
			
			// 遍历每一个栈帧的参数列表
			std::cerr << std::endl << "paras >";
			auto& list = frame->paras_info.act_para_list;
			auto size_list = list.size();
			for (int i = 0; i < size_list; ++i) {
				std::cout << i << '=' << list[i]->toEchoString() << ',' << std::ends;
			}

			// 是否是强作用域
			std::cerr << std::endl << "isStrongField >";
			auto& isStrongField = frame->_strong_hold;
			std::cout << (isStrongField ? "true" : "false") << std::endl;
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
