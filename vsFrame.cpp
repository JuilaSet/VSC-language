#include "pch.h"
#include "vsFrame.h"

// 获取外部运行上下文
RunTimeStackFrame_ptr _StackFrame::get_current_function_context() {
	if (paras_info.act_para_list.empty()) {
		return nullptr;
	}
	auto data_obj = paras_info.act_para_list[0];

	// 出错, 说明call传递的第一个参数不是函数类型
	assert(data_obj->getType() == DataType::FUNCTION);

	// 得到函数对象的运行环境(向子类转换可能出错????)
	auto function_obj = reinterpret_cast<IEvaluable*>(&*data_obj.get());
	auto runtime_ptr = function_obj->get_runtime_ctx_ptr();
	return runtime_ptr;
}