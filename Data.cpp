#include "pch.h"
#include "Data.h"

// 执行
int FunctionData::eval(vsEvaluator& evalor, int argc) {
	// 跳转block
	auto block_id = toIndex();
	auto vm = evalor.getVM();
	auto block_ptr = vm->get_block_ref(block_id);

#if	CHECK_Compiler_Field_NEW_BLK
	std::cerr << __LINE__ << " BLOCK_Field: " << (block_ptr->strong_hold() ? "T" : "F") << std::endl;
#endif

	evalor.load_block(block_ptr, argc);

	// 
	runtime_context_ptr = evalor.current_stk_frame();
	return 0;
}

// 获取当前运行上下文
RunTimeStackFrame_ptr FunctionData::get_runtime_ptr() {
	return runtime_context_ptr;
}