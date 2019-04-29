#include "pch.h"
#include "vsVirtualMachine.h"

data_ptr vsVirtualMachine::run(vsThread::Session<std::string, data_ptr>& sess, vsTool::id_t eval_id, std::string aim){
	if (!this->_inited)throw vsVm_not_inited_exception(_id);

	// session start
	return sess.eval(eval_id, aim);
}
