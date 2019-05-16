#include "pch.h"
#include "vsVirtualMachine.h"

data_ptr vsVirtualMachine::run(vsTool::id_t eval_id, std::string aim){
	// 设置会话
	vsThread::Session<std::string, data_ptr> sess(_taskGraphic);	// 开启会话

	if (!this->_inited)throw vsVm_not_inited_exception(_id);

	// session start, aim表示最后要求的结点值
	return sess.eval(eval_id, aim);
}
