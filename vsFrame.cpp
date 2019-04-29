#include "pch.h"
#include "vsFrame.h"
#include "Data.h"
#include "vsfunction.h"
#include "vsContainer.h"

// 构造
_StackFrameParasInfo::_StackFrameParasInfo() : act_para_container(new vsIdentifierContainer) { }

// 初始化
void _StackFrameParasInfo::init() {
	act_para_container->clearAll();
	pass_paras_list.clear();
}

_StackFrame::_StackFrame() : local_var_table(new vsIdentifierContainer) { }

// 获取外部运行上下文
RunTimeStackFrame_ptr _StackFrame::get_current_function_context() {
	if (paras_info.pass_paras_list.empty()) {
		return nullptr;
	}
	auto data_obj = paras_info.pass_paras_list[0];

	// 出错, 说明call传递的第一个参数不是函数类型
	if (data_obj->getType() == DataType::DELEGATION) {
		data_obj = IDelegation::cast_delegation_ptr(data_obj)->container_find();
	}
	assert(data_obj->getType() == DataType::FUNCTION);

	// 得到函数对象的运行环境(向子类转换可能出错????)
	auto function_obj = reinterpret_cast<IEvaluable*>(&*data_obj.get());
	auto runtime_ptr = function_obj->get_runtime_ctx_ptr();
	return runtime_ptr;
}

// 形参结合
void _StackFrame::pass_paras(block_ptr innerblock) {
	auto& f_list = innerblock->get_form_paras_list_ref();
	auto& p_list = paras_info.pass_paras_list; // 第一个参数是函数下标
//	auto& act_list = paras_info.act_para_list;
	auto act_list = paras_info.act_para_container;	// 实参结合表<id -> data_ptr>
	auto begin = 0;
	auto fend = f_list.size();
	auto pend = p_list.size();
	for (int it = begin + 1;; ++it) {
		auto data = NULL_DATA::null_data;
		if (it == pend) {

		}
		else if (p_list.size() > it) {
			data = p_list[it];
		}
		if (it - 1 == fend) {
			break;
		}
#if CHECK_Eval
		std::cout << "形参结合: " <<
			f_list[it - 1] << "->" << 
			data->toEchoString() << std::endl;
#endif
		act_list->assign(data_ptr(new StringData(f_list[it - 1])), data);
	}
}