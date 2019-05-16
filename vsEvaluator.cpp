#include "pch.h"
#include "vsEvaluator.h"

// vsEvaluator

// 在此之前要使用push_next_temp_paras_info(), 防止exit_block的时候出错
void vsEvaluator::load_block(int enter_point, int paras_count)
{
	auto block = this->_vm->get_block_ref(enter_point, _process_id);

	// 设置并传递栈帧
	_push_and_setting_frame(this->_block_ptr, block, paras_count);
	
	// 这里之后是内层
	_flag_has_instruct = true;

	// 进入block, 从头开始执行
	this->_instruct_ptr = &block->instruct();
	this->_block_ptr = block;
	ipc = -1; // 跳转后变成0
}

// 退出block
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
		block_ptr lastblock = _vm->get_block_ref(_return_blk_id, _process_id);

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

		// 形参结合
		stkframe->pass_paras(curblock);
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

// 获取外部数据(没有找到返回空对象)
data_ptr vsEvaluator::_get_extern_data(std::string index) {
	auto ptr = _extern_datas.find(data_ptr(new StringData(index)));
	if (ptr == nullptr) {
		return NULL_DATA::null_data;
	}
	return ptr;
}

// 设置外部数据(会覆盖原有数据)
void vsEvaluator::_add_extern_data(std::string index, data_ptr data) {
	_extern_datas.assign(data_ptr(new StringData(index)), data);
}

// 设置返回数据
void vsEvaluator::_set_return_data(data_ptr ret_data) {
	this->_ret_data = ret_data;
}

// 根据index获取data对象
data_ptr vsEvaluator::_find_data(std::string index, local_var_container_ptr& table_ret) {
	data_ptr ret;
	bool flag_found = false;

	// 查看是否是局部变量
	auto frame = current_stk_frame();
	auto var_table = frame->local_var_table;
	auto fit = var_table->find(data_ptr(new StringData(index)));
	if (fit != nullptr) {
		ret = fit;
		table_ret = var_table;
		flag_found = true;
	}

	// 查看作用域链
	if (!flag_found) {
		auto runtime_context_ptr = frame->get_current_function_context();
		while (runtime_context_ptr != nullptr) {
			// 查看外部环境的局部变量表
			auto table = runtime_context_ptr->local_var_table;
			auto _act_para_list = frame->paras_info.act_para_container;
			auto fit = table->find(data_ptr(new StringData(index)));
			if (fit != nullptr) {
				ret = fit;
				table_ret = table;
				flag_found = true;
				break;
			}
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
			auto var_table = frame->local_var_table;
			auto fit = var_table->find(data_ptr(new StringData(index)));
			if (fit != nullptr) {
				ret = fit;
				table_ret = var_table;
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
data_ptr vsEvaluator::_find_para(std::string index, para_var_container_ptr& act_paras_table_ret) {
	// 查看是否是该函数的参数
	bool flag_found = false;
	data_ptr ret = nullptr;
	auto frame = current_stk_frame();
	auto _act_para_list = frame->paras_info.act_para_container;
	// 存在就返回这个data
	auto fit = _act_para_list->find(data_ptr(new StringData(index)));
	if (fit != nullptr) {
		ret = fit;
		act_paras_table_ret = _act_para_list;
		flag_found = true;
	}

	// 查看是否是闭包的外部实参
	if (!flag_found) {
		auto runtime_context_ptr = frame->get_current_function_context();
		while (runtime_context_ptr != nullptr) {
			auto _act_para_list = runtime_context_ptr->paras_info.act_para_container;
			auto fit = _act_para_list->find(data_ptr(new StringData(index)));
			if (fit != nullptr) {
				ret = fit;
				act_paras_table_ret = _act_para_list;
				flag_found = true;
				break;
			}
			// 找到强作用域为止
			else if (runtime_context_ptr->_strong_hold) {
#if CHECK_Eval
				std::cerr << __LINE__ << "\tPARA DRF FAILED" << std::endl;
#endif
				break;
			}
			// 继续
			runtime_context_ptr = runtime_context_ptr->get_current_function_context();
		}
	}
	// 不存在就返回NULL_DATA
	return (flag_found ? ret : nullptr);
}

// new def
void vsEvaluator::new_regist_identity(std::string index, data_ptr d) {
	assert(!_stk_frame.empty());
	auto frame = current_stk_frame();
	auto var_table = frame->local_var_table;

	// 检查是否存在这个index
	auto key = data_ptr(new StringData(index));
	var_table->assign(key, d);
}

// new assign
bool vsEvaluator::new_set_data(std::string index, data_ptr d, bool isCopyMode) {
	assert(!_stk_frame.empty());
	local_var_container_ptr table_ret = nullptr;
	auto data = _find_data(index, table_ret);
	if (data == nullptr) {
#if CHECK_Eval
		std::cerr << std::endl << __LINE__ << "\tNEW STYLE ASSIGN FAILED" << std::endl;
#endif
		return false;
	}
	auto key = data_ptr(new StringData(index));
	
	// 是否是copy
	if (isCopyMode)
		table_ret->cp(key, d);		// 复制值
	else
		table_ret->assign(key, d);	// 修改句柄的指向
	return true;
}

// new drf (返回data的句柄)
data_ptr vsEvaluator::new_get_data(std::string index) {
	assert(!_stk_frame.empty());
	local_var_container_ptr table_ret = nullptr;
	auto data = _find_data(index, table_ret);
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
	frame->temp_stkframe.backParasInfo().pass_paras_list.push_back(data);
}

// para assign
bool vsEvaluator::para_assign_data(std::string index, data_ptr data, bool isCopyMode) {
	// 只找寻一层
	assert(!_stk_frame.empty());
	para_var_container_ptr act_paras_table_ret = nullptr;
	auto p_data = _find_para(index, act_paras_table_ret);
	if (p_data == nullptr) {
#if CHECK_Eval
		std::cerr << std::endl << __LINE__ << "\tPARA ASSIGN FAILED" << std::endl;
#endif
		// 不存在
		return false;
	}
	auto key = data_ptr(new StringData(index));
	// 是否是copy
	if (isCopyMode)
		act_paras_table_ret->cp(key ,data);
	else
		act_paras_table_ret->assign(key, data);
	return true;
}

// para drf
data_ptr vsEvaluator::para_get_data(std::string index) {
	assert(!_stk_frame.empty());
	para_var_container_ptr act_paras_table_ret = nullptr;
	auto p_data = _find_para(index, act_paras_table_ret);
	if (p_data == nullptr) {
#if CHECK_Eval
		std::cerr << std::endl <<  __LINE__ << "\tPARA DRF FAILED" << std::endl;
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
		std::cerr << "Data info:" << std::endl;
		//  遍历所有的frame
		for (auto& frame : _stk_frame) {
			std::cerr << "[";
			// 遍历全部栈帧的操作数栈
			std::cerr << "data >" << std::ends;
			auto& stk = frame->stk;
			for (auto d : stk) {
				std::cerr << d->getTypeName() << ": " << d->toEchoString() << ',' << std::ends;
			}

			// 遍历每一个栈帧的局部变量表
			std::cerr << std::endl << "identifier >";
			auto table = frame->local_var_table;
			table->for_each([](auto key, auto data) {
				std::cout << key << '=' << data->toEchoString() << ',' << std::ends;
				return true;
			});

			// 遍历每一个栈帧的参数列表
			std::cerr << std::endl << "paras >";
			auto list = frame->paras_info.act_para_container;
			list->for_each([](auto key, auto data) {
				std::cout << key << '=' << data->toEchoString() << ',' << std::ends;
				return true;
			});

			// 是否是强作用域
			std::cerr << std::endl << "isStrongField >";
			auto isStrongField = frame->_strong_hold;
			std::cout << (isStrongField ? "true" : "false") << std::endl;
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
			auto table = frame->local_var_table;
			table->for_each([] (auto key, auto data){
				std::cout << key << '=' << data->toEchoString() << ',' << std::ends;
				return true;
			});
			
			// 遍历每一个栈帧的参数列表
			std::cerr << std::endl << "paras >";
			auto list = frame->paras_info.act_para_container;
			list->for_each([](auto key, auto data) {
				std::cout << key << '=' << data->toEchoString() << ',' << std::ends;
				return true;
			});

			// 是否是强作用域
			std::cerr << std::endl << "isStrongField >";
			auto isStrongField = frame->_strong_hold;
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
