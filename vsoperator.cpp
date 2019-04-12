#include "pch.h"
#include "vsoperator.h"

void OPERATOR::ERROR(vsEval_ptr eval) {
	std::cerr << "ERROR" << std::endl;
	eval->stop();
}

void OPERATOR::ABORT(vsEval_ptr eval)		// 停止
{
	auto& stk = eval->current_stk_frame()->stk;

	// 取出返回值
	assert(!stk.empty());
	data_ptr n = eval->pop();
	assert(n->getType() == DataType::NUMBER);
	int a = n->toNumber();

	eval->stop(a);
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::ABORT" << std::endl;
#endif
}

void OPERATOR::NOP(vsEval_ptr eval)		// 什么都不做
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::NOP" << std::endl;
#endif
}

void OPERATOR::PUSH_POS(vsEval_ptr eval)
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::PUSH_POS " << eval->ipc << std::endl;
#endif

	unsigned int addr = eval->ipc;
	assert(eval->_instruct_ptr->size() > addr);
	eval->push(data_ptr(new NumData(DataType::OPERA_ADDR, addr)));
}

void OPERATOR::REVERSE_TOP(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::REVERSE_TOP " << std::endl;
#endif
	eval->reverse_top();
}

void OPERATOR::ADD(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::ADD ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	// 根据第一个参数的类型转换
	assert(!stk.empty());
	data_ptr n1 = eval->pop();

	assert(!stk.empty());
	data_ptr n2 = eval->pop();

	data_ptr d_temp = n2->add(n1);
	eval->push(d_temp);
#if CHECK_Eval 
	std::cerr << n1->toEchoString() << " + " << n2->toEchoString() << " = " << d_temp->toEchoString() << std::endl;
#endif
};

void OPERATOR::SUB(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::SUB ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr n1 = eval->pop();
	assert(n1->getType() == DataType::NUMBER);
	int a1 = n1->toNumber();

	assert(!stk.empty());
	data_ptr n2 = eval->pop();
	assert(n2->getType() == DataType::NUMBER);
	int a2 = n2->toNumber();
	// newed
	data_ptr d_temp = data_ptr(new NumData(DataType::NUMBER, a2 - a1));
	eval->push(d_temp);
#if CHECK_Eval 
	std::cerr << a2 << " - " << a1 << " = " << d_temp->toNumber() << std::endl;
#endif
}

void OPERATOR::NOT(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::NOT ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr n1 = eval->pop();
	assert(n1->getType() == DataType::NUMBER);
	int a1 = n1->toNumber();

	data_ptr d_temp = data_ptr(new NumData(DataType::NUMBER, a1 == 0));
	eval->push(d_temp);
#if CHECK_Eval 
	std::cerr << a1 << " = " << d_temp->toNumber() << std::endl;
#endif
}

void OPERATOR::EQ(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::EQ ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr n1 = eval->pop();

	assert(!stk.empty());
	data_ptr n2 = eval->pop();

	// newed
	data_ptr d_temp = data_ptr(new NumData(DataType::NUMBER, n2->eq(n1)));
	eval->push(d_temp);
#if CHECK_Eval 
	std::cerr << d_temp->toEchoString() << std::endl;
#endif
}

void OPERATOR::G(vsEval_ptr eval)
{
#if CHECK_Eval
	std::cerr << __LINE__ << "\tOPCODE::G ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr n1 = eval->pop();
	
	assert(!stk.empty());
	data_ptr n2 = eval->pop();

	// newed
	data_ptr d_temp = data_ptr(new NumData(DataType::NUMBER, n1->g(n2)));
	eval->push(d_temp);
#if CHECK_Eval
	std::cerr << d_temp->toEchoString() << std::endl;
#endif
}

void OPERATOR::L(vsEval_ptr eval)
{
#if CHECK_Eval
	std::cerr << __LINE__ << "\tOPCODE::L ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr n1 = eval->pop();

	assert(!stk.empty());
	data_ptr n2 = eval->pop();

	// newed
	data_ptr d_temp = data_ptr(new NumData(DataType::NUMBER, n1->g(n2)));
	eval->push(d_temp);
#if CHECK_Eval
	std::cerr << d_temp->toEchoString() << std::endl;
#endif
}

void OPERATOR::POP(vsEval_ptr eval)		// 栈顶弹出丢弃数据
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::POP ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
#if CHECK_Eval 
	std::cerr << stk.back()->toEchoString() << std::endl;
#endif
	eval->pop();
}

void OPERATOR::CAST_NUMBER(vsEval_ptr eval)
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::CAST_NUMBER ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr d = eval->pop();
	assert(d->getType() != DataType::OPERA_ADDR);
	int num = d->toNumber();
	// newed
	eval->push(data_ptr(new NumData(DataType::NUMBER, num)));
#if CHECK_Eval 
	std::cerr << num << std::endl;
#endif
}

void OPERATOR::CAST_STRING(vsEval_ptr eval)
{
#if CHECK_Eval
	std::cerr << __LINE__ << "\tOPCODE::CAST_STRING ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr d = eval->pop();
	assert(d->getType() != DataType::OPERA_ADDR);
	std::string str = d->toString();
	eval->push(data_ptr(new StringData(str)));
#if CHECK_Eval
	std::cerr << str << std::endl;
#endif
}

void OPERATOR::CAST_BOOL(vsEval_ptr eval) {
#if CHECK_Eval
	std::cerr << __LINE__ << "\tOPCODE::CAST_BOOL ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr d = eval->pop();
	assert(d->getType() != DataType::OPERA_ADDR);
	int b = d->toBool();
	eval->push(data_ptr(new NumData(DataType::NUMBER, b)));
#if CHECK_Eval
	std::cerr << b << std::endl;
#endif
}

void OPERATOR::TYPENAME(vsEval_ptr eval) {
#if CHECK_Eval
	std::cerr << __LINE__ << "\tOPCODE::TYPENAME ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr d = eval->pop();
	assert(d->getType() != DataType::OPERA_ADDR);
	auto name = d->getTypeName();
	eval->push(data_ptr(new StringData(name)));
#if CHECK_Eval
	std::cerr << name << std::endl;
#endif
}

void OPERATOR::CMP(vsEval_ptr eval)
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::CMP _f= ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr d1 = eval->pop();
	assert(!stk.empty());
	data_ptr d2 = eval->pop();
	eval->_f[0] = true;
	eval->_f[1] = true;
	if (d1->eq(d2)) {
		eval->_f[0] = false;
		eval->_f[1] = false;
	}
	else {
		if (d1->l(d2)) {
			// eval->_f[1] = true;
			eval->_f[0] = false;
		}
		else {
			// eval->_f[0] = true;
			eval->_f[1] = false;
		}
	}
	eval->push(eval->null_data);	// 返回void
#if CHECK_Eval 
	std::cerr << eval->_f[0] << eval->_f[1] << eval->_f[2] << std::endl;
#endif
}

void OPERATOR::TEST(vsEval_ptr eval)
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::TEXT _f[2] = ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr d = eval->pop();
	eval->_f[2] = d->toBool();
#if CHECK_Eval 
	std::cerr << eval->_f[2] << std::endl;
#endif
}

void OPERATOR::JE(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::JE ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	if (!(eval->_f[0] || eval->_f[1])) {
		assert(!stk.empty());
		data_ptr d = eval->pop();
		assert(d->getType() == DataType::OPERA_ADDR);
		eval->ipc = d->toAddr() - 1;
#if CHECK_Eval 
		std::cerr << d->toAddr() << std::endl;
#endif
	}
}

void OPERATOR::JNE(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::JNE ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	if (eval->_f[0] && eval->_f[1]) {
		assert(!stk.empty());
		data_ptr d = eval->pop();
		assert(d->getType() == DataType::OPERA_ADDR);
		eval->ipc = d->toAddr() - 1;
#if CHECK_Eval 
		std::cerr << d->toAddr() << std::endl;
#endif
	}
}

void OPERATOR::JG(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::JG ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	if (eval->_f[0] && !eval->_f[1]) {
		assert(!stk.empty());
		data_ptr d = eval->pop();
		assert(d->getType() == DataType::OPERA_ADDR);
		eval->ipc = d->toAddr() - 1;
#if CHECK_Eval 
		std::cerr << d->toAddr() << std::endl;
#endif
	}
}

void OPERATOR::JL(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::JL ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	if (!eval->_f[0] && eval->_f[1]) {
		assert(!stk.empty());
		data_ptr d = eval->pop();
		assert(d->getType() == DataType::OPERA_ADDR);
		eval->ipc = d->toAddr() - 1;
#if CHECK_Eval 
		std::cerr << d->toAddr() << std::endl;
#endif
	}
}

void OPERATOR::JEG(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::JEG ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	if (!eval->_f[1]) {	// 不小于
		assert(!stk.empty());
		data_ptr d = eval->pop();
		assert(d->getType() == DataType::OPERA_ADDR);
		eval->ipc = d->toAddr() - 1;
#if CHECK_Eval 
		std::cerr << d->toAddr() << std::endl;
#endif
	}
}

void OPERATOR::JEL(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::JEL ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	if (!eval->_f[0]) {	// 不大于
		assert(!stk.empty());
		data_ptr d = eval->pop();
		assert(d->getType() == DataType::OPERA_ADDR);
		eval->ipc = d->toAddr() - 1;
#if CHECK_Eval 
		std::cerr << d->toAddr() << std::endl;
#endif
	}
}

void OPERATOR::JMP(vsEval_ptr eval)
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::JMP ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr d = eval->pop();
	assert(d->getType() == DataType::OPERA_ADDR);
	eval->ipc = d->toAddr() - 1;
#if CHECK_Eval 
	std::cerr << d->toAddr() << std::endl;
#endif
}

void OPERATOR::JMP_TRUE(vsEval_ptr eval)
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::JMP_TRUE ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr d = eval->pop();
	assert(d->getType() == DataType::OPERA_ADDR);
	if (eval->_f[2]) {
		eval->ipc = d->toAddr() - 1;
#if CHECK_Eval 
		std::cerr << d->toAddr() << std::endl;
#endif
	}
	else {
#if CHECK_Eval 
		std::cerr << "NO JMP" << std::endl;
#endif
	}
}

void OPERATOR::JMP_FALSE(vsEval_ptr eval)
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::JMP_FALSE ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr d = eval->pop();
	assert(d->getType() == DataType::OPERA_ADDR);
	if (!eval->_f[2]) {
		eval->ipc = d->toAddr() - 1;
#if CHECK_Eval 
		std::cerr << d->toAddr() << std::endl;
#endif
	}
}

void OPERATOR::COUNT(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::COUNT ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr n = eval->pop();
	assert(n->getType() == DataType::NUMBER);
	eval->ecx = n->toNumber();
#if CHECK_Eval 
	std::cerr << n->toNumber() << std::endl;
#endif
}

void OPERATOR::TIME_BEGIN(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::TIME_BEGIN" << std::endl;
#endif
	eval->time_point = std::chrono::system_clock::now();
}

void OPERATOR::TIME_END(vsEval_ptr eval) {
#if CHECK_Eval
	std::cerr << __LINE__ << "\tOPCODE::TIME_END = ";
#endif
	std::chrono::system_clock::time_point t1 = eval->time_point;
	std::chrono::system_clock::time_point t2 = std::chrono::system_clock::now();
	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	std::stringstream s;
	s << time_span.count();
	std::string str;
	s >> str;

	eval->push(data_ptr(new StringData(str)));
#if CHECK_Eval 
	std::cerr << str << std::endl;
#endif
}

void OPERATOR::ECX(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::ECX " << std::endl;
#endif
	eval->push(data_ptr(new NumData(DataType::NUMBER, eval->ecx)));	// 计数器大小
}

void OPERATOR::NUL(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::NUL " << std::endl;
#endif
	eval->push(eval->null_data);
}

void OPERATOR::REPT(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::REPT ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr addr = eval->pop();
	assert(addr->getType() == DataType::OPERA_ADDR);
	if (eval->ecx > 0) {
		eval->ipc = addr->toAddr() - 1;
		eval->ecx--;
#if CHECK_Eval 
		std::cerr << addr->toAddr() << std::endl;
#endif
	}
#if CHECK_Eval 
	else {
		std::cerr << "END " << std::endl;
	}
#endif
}

void OPERATOR::ISNON(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::ISEND ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr d = eval->top();
	if (d->getType() == DataType::NON) {
		eval->_f[2] = 1;
	}
	else {
		eval->_f[2] = 0;
	}
#if CHECK_Eval 
	std::cerr << std::string(eval->_f[2] ? "true" : "false") << std::endl;
#endif
}

void OPERATOR::EQL(vsEval_ptr eval)
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::EQL ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr d1 = eval->pop();
	assert(!stk.empty());
	data_ptr d2 = eval->pop();
	if (d1 == d2) {
		eval->_f[0] = false;
		eval->_f[1] = false;
	}
#if CHECK_Eval 
	std::cerr << std::string((d1 == d2) ? "true" : "false") << std::endl;
#endif
}

void OPERATOR::NEQL(vsEval_ptr eval)
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::NEQL ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr d1 = eval->pop();
	assert(!stk.empty());
	data_ptr d2 = eval->pop();
	if (d1 == d2) {
		eval->_f[0] = true;
		eval->_f[1] = true;
	}
#if CHECK_Eval 
	std::cerr << std::string(!(d1 == d2) ? "true" : "false") << std::endl;
#endif
}

void OPERATOR::NEW_DEF(vsEval_ptr eval)
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tDEF ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	// push id push data def
	assert(!stk.empty());
	data_ptr d = eval->pop();
	assert(!stk.empty());
	data_ptr id = eval->pop();
	assert(id->getType() == DataType::ID_INDEX);
	eval->new_regist_identity(id->toIndex(), d);
	// 返回定义的data
	eval->push(d);
#if CHECK_Eval 
	std::cerr << id->toEchoString() << ":= " << d->toEchoString() << std::endl;
#endif
}

void OPERATOR::NEW_ASSIGN(vsEval_ptr eval)
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tASSIGN ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr value = eval->pop();
	assert(!stk.empty());
	data_ptr id = eval->pop();
	
	auto type = id->getType();
	if (type == DataType::ID_INDEX) {
		// 当做局部变量赋值
		bool local_assign_success = eval->new_set_data(id->toIndex(), value);
		assert(local_assign_success);
	}
	else if (type == DataType::PARA_INDEX) {
		// 当做参数变量赋值
		bool paras_assign_success = eval->para_assign_data(id->toIndex(), value);
		assert(paras_assign_success);
	}
	else {
		assert(false);
	}

	// 返回赋值的data
	eval->push(value);
#if CHECK_Eval 
	std::cerr << id->toEchoString() << ":= " << value->toEchoString() << std::endl;
#endif
}

void OPERATOR::NEW_DRF(vsEval_ptr eval)
{
#if CHECK_Eval
	std::cerr << __LINE__ << "\tDRF ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr d = eval->pop();
	d = (eval)->new_get_data(d->toIndex());
#if CHECK_Eval 
	std::cerr << d->toEchoString() << std::endl;
#endif
	// 返回解引用的指针
	eval->push(d);
}

// 对象所指向的内容进行浅复制
void OPERATOR::CP(vsEval_ptr eval) {

#if CHECK_Eval 
	std::cerr << __LINE__ << "\tCP ";
#endif
	auto& stk = eval->current_stk_frame()->stk;

	assert(!stk.empty());
	data_ptr value = eval->pop();
	assert(!stk.empty());
	data_ptr id = eval->pop();

	auto type = id->getType();
	if (type == DataType::ID_INDEX) {
		// 当做局部变量赋值
		bool local_assign_success = eval->new_set_data(id->toIndex(), value, true);
		assert(local_assign_success);
	}
	else if (type == DataType::PARA_INDEX) {
		// 当做参数变量赋值
		bool paras_assign_success = eval->para_assign_data(id->toIndex(), value, true);
		assert(paras_assign_success);
	}
	else {
		assert(false);
	}

	// 返回赋值的data
	eval->push(value);
#if CHECK_Eval 
	std::cerr << id->toEchoString() << ":= " << value->toEchoString() << std::endl;
#endif
}

void OPERATOR::RET(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::RET ";
#endif
	auto frame = eval->current_stk_frame();

	// 获取下标
	auto& stk = frame->stk;
	assert(!stk.empty());

	// 得到返回值, 可以是任何类型
	data_ptr ret_data = eval->pop();

	// 退出block
	assert(eval->_block_ptr);
	eval->exit_block();

	// 将返回值放入上一层的stk
	eval->push(ret_data);

#if CHECK_Eval 
	std::cerr << ret_data->toEchoString() << std::endl;
#endif
}

void OPERATOR::BREAK(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::BREAK " << std::endl;
#endif
	// 退出block
	assert(eval->_block_ptr);
	eval->exit_block();
}

void OPERATOR::ENCLOSED(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::ENCLOSED " << std::endl;
#endif
	assert(!eval->isGlobal());
	auto frame = eval->current_stk_frame();

	// 当前环境传递给function对象
	assert(!frame->stk.empty());
	auto function_obj = eval->top();

	assert(function_obj->getType() == DataType::FUNCTION);
	IEvaluable* evalable = reinterpret_cast<IEvaluable*>(&*function_obj);
	evalable->set_runtime_ctx_ptr(frame);
}


// 分配临时实参列表, 为接下来传递参数做准备
void OPERATOR::CALL_BLK_BEGIN(vsEval_ptr eval) {
	// 注意: call Block的时候还是在栈外面
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::CALL_BLK_BEGIN ";
#endif
	auto frame = eval->current_stk_frame();
	frame->push_next_temp_paras_info();
#if CHECK_Eval 
	std::cerr << std::endl;
#endif
}

void OPERATOR::CALL_BLK(vsEval_ptr eval)
{
	// 注意: call Block的时候还是在栈外面
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::CALL_BLK ";
#endif

	auto frame = eval->current_stk_frame();

	// call的参数个数
	assert(!frame->stk.empty());
	auto count = eval->pop()->toNumber();

	// call列表的第一个参数, 索引将要传递给下一次的参数
	auto& act_list = frame->temp_stkframe.backParasInfo().act_para_list;
	assert(!act_list.empty());

	// 获取function对象, 设置运行时上下文
	auto function_obj = act_list[0];
	assert(function_obj->getType() == DataType::FUNCTION);
	reinterpret_cast<IEvaluable*>(&*function_obj)->eval(*eval, count);
}

// 传递实参给形参, 注册data到该临时帧的形参表中
void OPERATOR::PARA_PASS(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::PARA_PASS ";
#endif

	data_ptr data = eval->pop();

	// 将实参PUSH到当前栈帧的临时实参列表中, 会在push_frame中传递信息给新建的frame
	assert(!eval->_stk_frame.empty());
	eval->para_pass_data(data);

#if CHECK_Eval 
	std::cerr << data->toEchoString() << std::endl;
#endif
}

// 对形参解引用, 从该帧的形参表中查询
void OPERATOR::PARA_DRF(vsEval_ptr eval) {
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tOPCODE::PARA_DRF ";
#endif
	auto& stk = eval->current_stk_frame()->stk;
	
	assert(!eval->_stk_frame.empty());
	auto frame = eval->current_stk_frame();

	// 获取下标
	assert(!stk.empty());
	data_ptr index_d = eval->pop();
	size_t index = index_d->toIndex();

	// 从当前栈帧的实参列表中查找参数
	data_ptr data = eval->para_get_data(index);

	// 返回解引用的指针
	eval->push(data);

#if CHECK_Eval 
	std::cerr << index << std::endl;
#endif
}

// 在编译时, local_begin符号用于确定作用域, 执行时这个功能留在之后用于优化代码
void OPERATOR::LOCAL_BEGIN(vsEval_ptr eval)
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tLOCAL_BEGIN ";
#endif
	// 暂时什么都不做 []][
	// eval->push_frame();
	// eval->blk_stk.push_back(stk.size());
#if CHECK_Eval 
	std::cerr << "frame_vec_size= " << eval->_stk_frame.size() << std::endl;
	//		std::cerr << __LINE__ << "\tBLK_STK size= " << eval->blk_stk.size() << std::endl;
#endif
}

void OPERATOR::LOCAL_END(vsEval_ptr eval)
{
#if CHECK_Eval 
	std::cerr << __LINE__ << "\tLOCAL_END ";
#endif
	assert(!eval->_stk_frame.empty());
	eval->exit_block();
	//		eval->pop_frame();
	//		assert(!eval->blk_stk.empty());
	//		eval->blk_stk.pop_back();
#if CHECK_Eval 
	std::cerr << "frame_vec_size= " << eval->_stk_frame.size() << std::endl;
	//		std::cerr << __LINE__ << "\tBLK_STK size= " << eval->blk_stk.size() << std::endl;
#endif
}

// 由于LOCAL_END的时候会自动回收栈内资源, 这个功能留在之后用于优化代码
void OPERATOR::SHRINK(vsEval_ptr eval)
{
#if CHECK_Eval
	std::cerr << __LINE__ << "\tSHRINK []][" << std::endl;
#endif
	// auto& stk = eval->current_stk_frame()->stk;

	// unsigned int size = eval->blk_stk.back();
	// 现在的大小到之前的大小之差
	// int dirf = stk.size() - size;
	// auto end = stk.end();
	// stk.erase(end - dirf, end);
#if CHECK_Eval
		// std::cerr << "list_size= " << eval->_var_list.size() << std::endl;
#endif
}
