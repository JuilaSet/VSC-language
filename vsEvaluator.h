#pragma once
#include <cstdlib>
#include <cassert>

#include <vector>
#include <set>
#include <string>
#include <memory>
#include <sstream>
#include <functional>
#include <chrono>

#include "Lexer.h"

#define CHECK_Eval true
#define CHECK_Eval_command false

// 
// 如果要添加OPERATOR, 先在OPCODE中添加, 再在OPERATOR类中注册方法, 最后在getBasicCommandOfString中添加字符串转换
//

class vsEvaluator;
enum class OPCODE :int {
	ABORT = -1,	// 停止
	NOP,		// 什么都不做
	PUSH_POS,	// 当前opcode地址入栈
	POP,		// 栈顶弹出丢弃数据
	
	// 转换
	CAST_NUMBER,	// 转换为number型
	CAST_STRING,	// 转换为字符串
	CAST_BOOL,		// 转换为bool型
	REVERSE_TOP,	// 将栈顶两个数据交换

	// 比较
	ISNON,		// 是否遇到了non, 返回原来的值, 将bool类型放入_f[3]
	EQL,		// 等于比较
	NEQL,		// 不等于比较
	CMP,		// 比较
	TEST,		// 将bool保存到_f[3]
	JE,			// 等于跳转
	JNE,		// 不等于跳转
	JG,			// 大于跳转
	JL,			// 小于跳转
	JEG,		// 大于等于跳转
	JEL,		// 小于等于跳转
	JMP,		// 跳转, 到n
	JMP_TRUE,	// _f[2]为true跳转
	JMP_FALSE,	// _f[2]为false跳转
	REPT,		// 参数(addr), 返回n次
	COUNT,		// 参数(number), 设置ecx

	CALL,		// 将栈顶地址位置保存到temp_addr, 将当前地址位置入栈, ipc跳转到temp_addr
	RET,		// 跳转到栈顶地址位置, 不自动-1

	// 特殊立即数
	TIME_BEGIN,		// 当前时间
	TIME_END,		// 计时结束
	ECX,			// 获取计数器大小
	NUL,			// 空对象

	LOCAL_BEGIN,	// 生成局部变量栈
	LOCAL_END,		// POP局部变量栈

	// 运算
	DRF,		// 解除引用
	DEF,		// 变量定义
	ASSIGN,		// 变量赋值
	ADD,		// 栈前两个元素相加
	SUB,		// 栈前两个元素相减
	STRCAT,		// 连接字符串

	// 关系运算
	NOT,		// 非
	EQ,			// 等于
	L,			// 小于
	G,			// 大于

	// 内存管理
	SHRINK	// 恢复栈大小
};

// Command
using pOperaFunc_t = std::function<void(vsEvaluator*)>;

class Command {
	friend class vsEvaluator;

#if CHECK_Eval_command
	std::string _name;
#endif

protected:
	friend class CommandHelper;
public:
	pOperaFunc_t opera;

#if CHECK_Eval_command
	Command(pOperaFunc_t func, const std::string name = "unknown") :opera(func), _name(name) { }
	inline const std::string& name() const {
		return _name;
	}
#else
	Command(pOperaFunc_t func) :opera(func) { }
#endif
};
using _command_set = std::vector<Command>;

// 用于调试模式下给command打上对应的标签
#if CHECK_Eval_command
#define COMMAND(name) Command(OPERATOR::name, #name)
#else
#define COMMAND(name) Command(OPERATOR::name)
#endif
//

// 
class Command;
class CommandHelper;
class vsVirtualMachine;
using vec_command_t = std::vector<Command>;
using data_ptr = std::shared_ptr<Data>;
using data_list_t = std::map<std::string, data_ptr>;
using new_data_list_t = std::vector<data_ptr>;

// 运行时栈帧
struct StackFrame {
	new_data_list_t local_var_table; // 局部变量表, 根据int做随机访问, 每次有新的标识符就向上增加
	std::vector<data_ptr> stk;		 // 操作数栈
	unsigned int ret;				 // 返回地址
};

// 堆栈虚拟机
class vsEvaluator
{
	// 友元类
	friend class CommandHelper;
	friend class Command;
	friend class OPERATOR;
	friend class vsVirtualMachine;

	vec_command_t* _instruct_ptr;						// 指令数组
	std::vector<StackFrame> _stk_frame;					// 栈帧

	// std::vector<unsigned int> blk_stk;					// 保存所在语句块数据栈大小恢复值

	const data_ptr null_data = data_ptr(new Data());	// null对象

	// 是否停止执行
	bool _stop;

	// 停止时, 虚拟机的返回值
	int _stop_val;

	unsigned int ipc;
	bool _f[3];		// [0]: 大于 [1]: 小于 [2]: 是否标记
	int ecx;

	// 计时器
	std::chrono::system_clock::time_point time_point;

	// 是否有设置instruct指针
	bool _flag_has_instruct;

public:
	vsEvaluator() :_stop_val(0) , _flag_has_instruct(false) { };
	vsEvaluator(vec_command_t* ptr) :_instruct_ptr(ptr) ,_stop_val(0) , _flag_has_instruct(true) { }

	~vsEvaluator();

	// 设置执行代码
	void setInstructPtr(vec_command_t* ins);

	// 清空运行时栈
	void clear_frame() {
		this->_stk_frame.clear();
	}

	// 初始化虚拟机
	void init() {
		_stop = false;
		ipc = 0;
		// 清空栈帧
		_stk_frame.clear();
	}

	// new def
	void new_regist_identity(size_t index, data_ptr d);

	// new assign
	bool new_set_data(size_t index, data_ptr d);
	
	// new drf
	data_ptr new_get_data(size_t index);

	// 创建并压入栈帧
	void push_frame();

	// 弹出局部变量
	void pop_frame();

	// 单步执行
	virtual int step(vsVirtualMachine* vm);
	
	// 执行全部
	virtual int eval(vsVirtualMachine* vm);

	// 停止
	void stop(int stop_val = 0) { _stop = true; _stop_val = stop_val; }

	// 是否停止
	bool isstop() { return _stop; }

	// 获取当前栈
	inline StackFrame& current_stk_frame() {
		return _stk_frame.back();
	}

	// 交换顶层两个元素
	inline void reverse_top() {
		auto& stk = current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d1 = stk.back();
		stk.pop_back();
		assert(!stk.empty());
		data_ptr d2 = stk.back();
		stk.pop_back();
		
		stk.emplace_back(d1);
		stk.emplace_back(d2);
	}

	// 弹出顶层元素
	inline data_ptr pop() {
		auto& stk = current_stk_frame().stk;

		assert(!stk.empty());
		auto d = stk.back();
		stk.pop_back();
		return d;
	}

	// 返回顶层元素
	inline data_ptr top() {
		auto& stk = current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d = stk.back();
		return d;
	}

	// 压入元素
	inline void push(data_ptr d) {
		auto& stk = current_stk_frame().stk;

		stk.push_back(d);
	}
};

// 操作符对象
using vm_ptr = vsEvaluator *;
class OPERATOR {
public:
	static void ERROR(vm_ptr vm) {
		std::cerr << "ERROR" << std::endl;
		vm->stop();
	}

	static void ABORT(vm_ptr vm)		// 停止
	{
		auto& stk = vm->current_stk_frame().stk;

		// 取出返回值
		assert(!stk.empty());
		data_ptr n = vm->pop();
		assert(n->getType() == DataType::NUMBER);
		int a = n->toNumber();

		vm->stop(a);
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::ABORT" << std::endl;
#endif
	}

	static void NOP(vm_ptr vm)		// 什么都不做
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::NOP" << std::endl;
#endif
	}

	static void PUSH_POS(vm_ptr vm)
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::PUSH_POS " << vm->ipc << std::endl;
#endif

		unsigned int addr = vm->ipc;
		assert(vm->_instruct_ptr->size() > addr);
		vm->push(data_ptr(new Data(DataType::OPERA_ADDR, addr)));
	}

	static void REVERSE_TOP(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::REVERSE_TOP " << std::endl;
#endif
		vm->reverse_top();
	}

	static void ADD(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::ADD ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr n1 = vm->pop();
		assert(n1->getType() == DataType::NUMBER);
		int a1 = n1->toNumber();

		assert(!stk.empty());
		data_ptr n2 = vm->pop();
		assert(n2->getType() == DataType::NUMBER);
		int a2 = n2->toNumber();

		// newed
		data_ptr d_temp = data_ptr(new Data(DataType::NUMBER, a1 + a2));
		vm->push(d_temp);
#if CHECK_Eval 
		std::cerr << a1 << " + " << a2 << " = " << d_temp->toNumber() << std::endl;
#endif
	};

	static void SUB(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::SUB ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr n1 = vm->pop();
		assert(n1->getType() == DataType::NUMBER);
		int a1 = n1->toNumber();

		assert(!stk.empty());
		data_ptr n2 = vm->pop();
		assert(n2->getType() == DataType::NUMBER);
		int a2 = n2->toNumber();
		// newed
		data_ptr d_temp = data_ptr(new Data(DataType::NUMBER, a2 - a1));
		vm->push(d_temp);
#if CHECK_Eval 
		std::cerr << a2 << " - " << a1 << " = " << d_temp->toNumber() << std::endl;
#endif
	}

	static void NOT(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::NOT ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr n1 = vm->pop();
		assert(n1->getType() == DataType::NUMBER);
		int a1 = n1->toNumber();

		data_ptr d_temp = data_ptr(new Data(DataType::NUMBER, a1 == 0));
		vm->push(d_temp);
#if CHECK_Eval 
		std::cerr << a1 << " = " << d_temp->toNumber() << std::endl;
#endif
	}

	static void EQ(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::EQ ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr n1 = vm->pop();

		assert(!stk.empty());
		data_ptr n2 = vm->pop();

		// newed
		data_ptr d_temp = data_ptr(new Data(DataType::NUMBER, *n2 == *n1));
		vm->push(d_temp);
#if CHECK_Eval 
		std::cerr << (n2 == n1) << std::endl;
#endif
	}

	static void G(vm_ptr vm)
	{
#if CHECK_Eval
		std::cerr << __LINE__ << "\tOPCODE::G ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr n1 = vm->pop();

		assert(!stk.empty());
		data_ptr n2 = vm->pop();
		// newed
		data_ptr d_temp = data_ptr(new Data(DataType::NUMBER, !(*n2 < *n1) && !(*n2 == *n1)));
		vm->push(d_temp);
#if CHECK_Eval
		std::cerr << (!(n2 < n1) && !(n2 == n1)) << std::endl;
#endif
	}

	static void L(vm_ptr vm)
	{
#if CHECK_Eval
		std::cerr << __LINE__ << "\tOPCODE::L ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr n1 = vm->pop();

		assert(!stk.empty());
		data_ptr n2 = vm->pop();
		// newed
		data_ptr d_temp = data_ptr(new Data(DataType::NUMBER, *n2 < *n1));
		vm->push(d_temp);
#if CHECK_Eval
		std::cerr << (n2 < n1) << std::endl;
#endif
	}

	static void STRCAT(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::STRCAT ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr n1 = vm->pop();
		assert(n1->getType() == DataType::STRING);
		std::string a1 = n1->toString();

		assert(!stk.empty());
		data_ptr n2 = vm->pop();
		assert(n2->getType() == DataType::STRING);
		std::string a2 = n2->toString();

		data_ptr d_temp = data_ptr(new Data(DataType::STRING, a2 + a1));
		vm->push(d_temp);
#if CHECK_Eval 
		std::cerr << a2 << " + " << a1 << " = " << d_temp->toEchoString() << std::endl;
#endif
	}

	static void POP(vm_ptr vm)		// 栈顶弹出丢弃数据
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::POP ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
#if CHECK_Eval 
		std::cerr << stk.back()->toEchoString() << std::endl;
#endif
		vm->pop();
	}

	static void CAST_NUMBER(vm_ptr vm)
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::CAST_NUMBER ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d = vm->pop();
		assert(d->getType() != DataType::OPERA_ADDR);
		int num = d->toNumber();
		// newed
		vm->push(data_ptr(new Data(DataType::NUMBER, num)));
#if CHECK_Eval 
		std::cerr << num << std::endl;
#endif
	}

	static void CAST_STRING(vm_ptr vm)
	{
#if CHECK_Eval
		std::cerr << __LINE__ << "\tOPCODE::CAST_STRING ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d = vm->pop();
		assert(d->getType() != DataType::OPERA_ADDR);
		std::string str = d->toString();
		vm->push(data_ptr(new Data(DataType::STRING, str)));
#if CHECK_Eval
		std::cerr << str << std::endl;
#endif
	}

	static void CAST_BOOL(vm_ptr vm) {
#if CHECK_Eval
		std::cerr << __LINE__ << "\tOPCODE::CAST_BOOL ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d = vm->pop();
		assert(d->getType() != DataType::OPERA_ADDR);
		int b = d->toBool();
		vm->push(data_ptr(new Data(DataType::NUMBER, b)));
#if CHECK_Eval
		std::cerr << b << std::endl;
#endif
	}

	static void CMP(vm_ptr vm)
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::CMP _f= ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d1 = vm->pop();
		assert(!stk.empty());
		data_ptr d2 = vm->pop();
		vm->_f[0] = true;
		vm->_f[1] = true;
		if (*d1 == *d2) {
			vm->_f[0] = false;
			vm->_f[1] = false;
		}
		else {
			if (*d1 < *d2) {
				// vm->_f[1] = true;
				vm->_f[0] = false;
			}
			else {
				// vm->_f[0] = true;
				vm->_f[1] = false;
			}
		}
		vm->push(vm->null_data);	// 返回void
#if CHECK_Eval 
		std::cerr << vm->_f[0] << vm->_f[1] << vm->_f[2] << std::endl;
#endif
	}

	static void TEST(vm_ptr vm)
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::TEXT _f[2] = ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d = vm->pop();
		vm->_f[2] = d->toBool();
#if CHECK_Eval 
		std::cerr << vm->_f[2] << std::endl;
#endif
	}

	static void JE(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::JE ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		if (!(vm->_f[0] || vm->_f[1])) {
			assert(!stk.empty());
			data_ptr d = vm->pop();
			assert(d->getType() == DataType::OPERA_ADDR);
			vm->ipc = d->toAddr() - 1;
#if CHECK_Eval 
			std::cerr << d->toAddr() << std::endl;
#endif
		}
	}

	static void JNE(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::JNE ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		if (vm->_f[0] && vm->_f[1]) {
			assert(!stk.empty());
			data_ptr d = vm->pop();
			assert(d->getType() == DataType::OPERA_ADDR);
			vm->ipc = d->toAddr() - 1;
#if CHECK_Eval 
			std::cerr << d->toAddr() << std::endl;
#endif
		}
	}

	static void JG(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::JG ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		if (vm->_f[0] && !vm->_f[1]) {
			assert(!stk.empty());
			data_ptr d = vm->pop();
			assert(d->getType() == DataType::OPERA_ADDR);
			vm->ipc = d->toAddr() - 1;
#if CHECK_Eval 
			std::cerr << d->toAddr() << std::endl;
#endif
		}
	}

	static void JL(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::JL ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		if (!vm->_f[0] && vm->_f[1]) {
			assert(!stk.empty());
			data_ptr d = vm->pop();
			assert(d->getType() == DataType::OPERA_ADDR);
			vm->ipc = d->toAddr() - 1;
#if CHECK_Eval 
			std::cerr << d->toAddr() << std::endl;
#endif
		}
	}

	static void JEG(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::JEG ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		if (!vm->_f[1]) {	// 不小于
			assert(!stk.empty());
			data_ptr d = vm->pop();
			assert(d->getType() == DataType::OPERA_ADDR);
			vm->ipc = d->toAddr() - 1;
#if CHECK_Eval 
			std::cerr << d->toAddr() << std::endl;
#endif
		}
	}

	static void JEL(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::JEL ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		if (!vm->_f[0]) {	// 不大于
			assert(!stk.empty());
			data_ptr d = vm->pop();
			assert(d->getType() == DataType::OPERA_ADDR);
			vm->ipc = d->toAddr() - 1;
#if CHECK_Eval 
			std::cerr << d->toAddr() << std::endl;
#endif
		}
	}

	static void JMP(vm_ptr vm)
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::JMP ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d = vm->pop();
		assert(d->getType() == DataType::OPERA_ADDR);
		vm->ipc = d->toAddr() - 1;
#if CHECK_Eval 
		std::cerr << d->toAddr() << std::endl;
#endif
	}

	static void JMP_TRUE(vm_ptr vm)
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::JMP_TRUE ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d = vm->pop();
		assert(d->getType() == DataType::OPERA_ADDR);
		if (vm->_f[2]) {
			vm->ipc = d->toAddr() - 1;
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

	static void JMP_FALSE(vm_ptr vm)
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::JMP_FALSE ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d = vm->pop();
		assert(d->getType() == DataType::OPERA_ADDR);
		if (!vm->_f[2]) {
			vm->ipc = d->toAddr() - 1;
#if CHECK_Eval 
			std::cerr << d->toAddr() << std::endl;
#endif
		}
	}

	static void COUNT(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::COUNT ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr n = vm->pop();
		assert(n->getType() == DataType::NUMBER);
		vm->ecx = n->toNumber();
#if CHECK_Eval 
		std::cerr << n->toNumber() << std::endl;
#endif
	}

	static void TIME_BEGIN(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::TIME_BEGIN" << std::endl;
#endif
		vm->time_point = std::chrono::system_clock::now();
	}

	static void TIME_END(vm_ptr vm) {
#if CHECK_Eval
		std::cerr << __LINE__ << "\tOPCODE::TIME_END = ";
#endif
		std::chrono::system_clock::time_point t1 = vm->time_point;
		std::chrono::system_clock::time_point t2 = std::chrono::system_clock::now();
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		std::stringstream s;
		s << time_span.count();
		std::string str;
		s >> str;

		vm->push(data_ptr(new Data(DataType::STRING, str)));
#if CHECK_Eval 
		std::cerr << str << std::endl;
#endif
	}

	static void ECX(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::ECX " << std::endl;
#endif
		vm->push(data_ptr(new Data(DataType::NUMBER, vm->ecx)));	// 计数器大小
	}

	static void NUL(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::NUL " << std::endl;
#endif
		vm->push(vm->null_data);	// 计数器大小
	}

	static void REPT(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::REPT ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr addr = vm->pop();
		assert(addr->getType() == DataType::OPERA_ADDR);
		if (vm->ecx > 0) {
			vm->ipc = addr->toAddr() - 1;
			vm->ecx--;
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
	
	static void ISNON(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::ISEND ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d = vm->top();
		if (d->getType() == DataType::NON) {
			vm->_f[2] = 1;
		}
		else {
			vm->_f[2] = 0;
		}
#if CHECK_Eval 
		std::cerr << std::string(vm->_f[2] ? "true" : "false") << std::endl;
#endif
	}

	static void EQL(vm_ptr vm)
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::EQL ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d1 = vm->pop();
		assert(!stk.empty());
		data_ptr d2 = vm->pop();
		if (d1 == d2) {
			vm->_f[0] = false;
			vm->_f[1] = false;
		}
#if CHECK_Eval 
		std::cerr << std::string((d1 == d2) ? "true" : "false") << std::endl;
#endif
	}

	static void NEQL(vm_ptr vm)
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::NEQL ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d1 = vm->pop();
		assert(!stk.empty());
		data_ptr d2 = vm->pop();
		if (d1 == d2) {
			vm->_f[0] = true;
			vm->_f[1] = true;
		}
#if CHECK_Eval 
		std::cerr << std::string(!(d1 == d2) ? "true" : "false") << std::endl;
#endif
	}

	static void NEW_DEF(vm_ptr vm)
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tDEF ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		// push id push data def
		assert(!stk.empty());
		data_ptr d = vm->pop();
		assert(!stk.empty());
		data_ptr id = vm->pop();
		assert(id->getType() == DataType::ID_INDEX);
		vm->new_regist_identity(id->toIndex(), d);
		// 返回定义的data
		vm->push(d);
#if CHECK_Eval 
		std::cerr << id->toEchoString() << ":= " << d->toEchoString() << std::endl;
#endif
	}

	static void NEW_ASSIGN(vm_ptr vm)
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tASSIGN ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr value = vm->pop();
		assert(!stk.empty());
		data_ptr id = vm->pop();
		assert(id->getType() == DataType::ID_INDEX);
		vm->new_set_data(id->toIndex(), value);
		// 返回赋值的data
		vm->push(value);
#if CHECK_Eval 
		std::cerr << id->toEchoString() << ":= " << value->toEchoString() << std::endl;
#endif
	}

	static void NEW_DRF(vm_ptr vm)
	{
#if CHECK_Eval
		std::cerr << __LINE__ << "\tDRF ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d = vm->pop();
		d = (vm)->new_get_data(d->toIndex());
#if CHECK_Eval 
		std::cerr << d->toEchoString() << std::endl;
#endif
		// 返回解引用的指针
		vm->push(d);
	}

	static void CALL(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::CALL ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr temp_addr = vm->pop();
		assert(temp_addr->getType() == DataType::OPERA_ADDR);
#if CHECK_Eval 
		std::cerr << temp_addr->toEchoString() << std::endl;
#endif
		unsigned int addr = vm->ipc;
		// 当前地址入栈
		assert(vm->_instruct_ptr->size() > addr);
		vm->push(data_ptr(new Data(DataType::OPERA_ADDR, addr)));
		// jmp
		vm->ipc = temp_addr->toAddr() - 1;
	}

	static void RET(vm_ptr vm) {
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tOPCODE::RET ";
#endif
		auto& stk = vm->current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr addr = vm->pop();
		assert(vm->_instruct_ptr->size() > addr->toAddr());
		// jmp
		vm->ipc = addr->toAddr();
#if CHECK_Eval 
		std::cerr << addr->toEchoString() << std::endl;
#endif
	}

	static void LOCAL_BEGIN(vm_ptr vm)
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tLOCAL_BEGIN ";
#endif
		// 加入栈帧
		vm->push_frame();
		// vm->blk_stk.push_back(stk.size());
#if CHECK_Eval 
		std::cerr << "frame_vec_size= " << vm->_stk_frame.size() << std::endl;
//		std::cerr << __LINE__ << "\tBLK_STK size= " << vm->blk_stk.size() << std::endl;
#endif
	}

	static void LOCAL_END(vm_ptr vm)
	{
#if CHECK_Eval 
		std::cerr << __LINE__ << "\tLOCAL_END ";
#endif
		assert(!vm->_stk_frame.empty());
		vm->pop_frame();
//		assert(!vm->blk_stk.empty());
//		vm->blk_stk.pop_back();
#if CHECK_Eval 
		std::cerr << "frame_vec_size= " << vm->_stk_frame.size() << std::endl;
//		std::cerr << __LINE__ << "\tBLK_STK size= " << vm->blk_stk.size() << std::endl;
#endif
	}

	// 由于LOCAL_END的时候会自动回收栈内资源, 删除这个功能
	static void SHRINK(vm_ptr vm) 
	{
#if CHECK_Eval
		std::cerr << __LINE__ << "\tSHRINK []][" << std::endl;
#endif
		// auto& stk = vm->current_stk_frame().stk;

		// unsigned int size = vm->blk_stk.back();
		// 现在的大小到之前的大小之差
		// int dirf = stk.size() - size;
		// auto end = stk.end();
		// stk.erase(end - dirf, end);
#if CHECK_Eval
		// std::cerr << "list_size= " << vm->_var_list.size() << std::endl;
#endif
	}
};

// Command
class CommandHelper {
public:
	static Command getBasicCommandOfString(std::string str);

	static Command getBasicOpera(pOperaFunc_t func) {
		return Command(func);
	}

	static Command getPushOpera(Data d) {
		return Command([=](vsEvaluator *vm) {
#if CHECK_Eval 
			std::cerr << __LINE__ << "\tOPCODE::PUSH " << d.toEchoString() << std::endl;
#endif
			vm->push(data_ptr(new Data(d)));
		}
#if CHECK_Eval_command
		, "PUSH " + d.toEchoString()
#endif 
		);
	};

	static Command getEchoOpera(std::ostream* ostm, std::string end = "\n") {
		return Command([=](vsEvaluator *vm) {
#if CHECK_Eval 
			std::cerr << __LINE__ << "\tOPCODE::ECHO ";
#endif
			auto& stk = vm->current_stk_frame().stk;
			assert(!stk.empty());
			data_ptr d = vm->top();
			std::string str = d->toEchoString();
#if CHECK_Eval 
			std::cerr << str << std::endl;
#endif
			*ostm << str << end << std::flush;
		}
#if CHECK_Eval_command
			, "ECHO"
#endif 
		);
	}
};
