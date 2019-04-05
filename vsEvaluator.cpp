#include "pch.h"
#include "vsEvaluator.h"

// vsEvaluator

void vsEvaluator::load_block(vsblock& block)
{
	_push_frame();

	_flag_has_instruct = true;

	// 进入block, 从头开始执行
	this->_instruct_ptr = &block.instruct();
	this->_block_ptr = &block;
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
		auto& lastblock = _vm->get_block_ref(_return_blk_id);

		// 恢复现场
		this->_instruct_ptr = &lastblock.instruct();
		this->_block_ptr = &lastblock;
		this->ipc = _return_addr;
	}
}

// 创建并压入栈帧
void vsEvaluator::_push_frame() {
#if CHECK_Eval
	std::cerr << "Push frame ";
#endif

	_StackFrame stkframe;

	// 出错, 可能是stk_frame在错误的地方push了栈帧
	assert( this->_block_ptr == nullptr && _stk_frame.empty() || 
		this->_block_ptr != nullptr && !_stk_frame.empty());

	// 如果不是顶层的block, 就告知返回地址, 设置当前返回地址 = 当前执行的地址
	if (this->_block_ptr != nullptr) {
		stkframe.ret.return_block_id = _block_ptr->id();
		stkframe.ret.return_addr = this->ipc;
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
