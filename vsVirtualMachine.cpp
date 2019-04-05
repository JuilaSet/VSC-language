#include "pch.h"
#include "vsVirtualMachine.h"

void vsVirtualMachine::run() {
	if (!this->_inited)throw vsVm_not_inited_exception(_id);
	// 新建解释器
	_eval_main = std::make_shared<vsEvaluator>(vsEvaluator(this));
	// 主线程进入入口点, 相当于直接 <call_blk _enter_point>
	_eval_main->load_block(get_block_ref(_enter_point));
	_eval_ret_value = _eval_main->eval();
}
