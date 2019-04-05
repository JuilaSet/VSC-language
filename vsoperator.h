#pragma once

// 
// 如果要添加OPERATOR, 先在OPCODE中添加, 再在OPERATOR类中注册方法, 最后在getBasicCommandOfString中添加字符串转换
//

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
	CALL_BLK,	// 跳转到指定的block处
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

// 操作符对象
using vsEval_ptr = vsEvaluator * ;
class OPERATOR {
public:
	static void ERROR(vsEval_ptr eval);

	static void ABORT(vsEval_ptr eval);

	static void NOP(vsEval_ptr eval);

	static void PUSH_POS(vsEval_ptr eval);

	static void REVERSE_TOP(vsEval_ptr eval);

	static void ADD(vsEval_ptr eval);

	static void SUB(vsEval_ptr eval);

	static void NOT(vsEval_ptr eval);

	static void EQ(vsEval_ptr eval);

	static void G(vsEval_ptr eval);

	static void L(vsEval_ptr eval);

	static void STRCAT(vsEval_ptr eval);

	static void POP(vsEval_ptr eval);

	static void CAST_NUMBER(vsEval_ptr eval);

	static void CAST_STRING(vsEval_ptr eval);

	static void CAST_BOOL(vsEval_ptr eval);

	static void CMP(vsEval_ptr eval);

	static void TEST(vsEval_ptr eval);

	static void JE(vsEval_ptr eval);

	static void JNE(vsEval_ptr eval);

	static void JG(vsEval_ptr eval);

	static void JL(vsEval_ptr eval);

	static void JEG(vsEval_ptr eval);

	static void JEL(vsEval_ptr eval);

	static void JMP(vsEval_ptr eval);

	static void JMP_TRUE(vsEval_ptr eval);

	static void JMP_FALSE(vsEval_ptr eval);

	static void COUNT(vsEval_ptr eval);

	static void TIME_BEGIN(vsEval_ptr eval);

	static void TIME_END(vsEval_ptr eval);

	static void ECX(vsEval_ptr eval);

	static void NUL(vsEval_ptr eval);

	static void REPT(vsEval_ptr eval);

	static void ISNON(vsEval_ptr eval);

	static void EQL(vsEval_ptr eval);

	static void NEQL(vsEval_ptr eval);

	static void NEW_DEF(vsEval_ptr eval);

	static void NEW_ASSIGN(vsEval_ptr eval);

	static void NEW_DRF(vsEval_ptr eval);

	// []][
	static void CALL(vsEval_ptr eval);

	static void RET(vsEval_ptr eval);

	static void CALL_BLK(vsEval_ptr eval);

	// 在编译时, local_begin符号用于确定作用域, 执行时这个功能留在之后用于优化代码
	static void LOCAL_BEGIN(vsEval_ptr eval);

	static void LOCAL_END(vsEval_ptr eval);

	// 由于LOCAL_END的时候会自动回收栈内资源, 这个功能留在之后用于优化代码
	static void SHRINK(vsEval_ptr eval);
};
