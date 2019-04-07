#pragma once

#define CHECK_Eval true
#define CHECK_Eval_command true

// 
class Command;
class CommandHelper;
class vsVirtualMachine;
class vsblock;
using vec_command_t = std::vector<Command>;
using data_ptr = std::shared_ptr<Data>;
using data_list_t = std::map<std::string, data_ptr>;
using new_data_list_t = std::vector<data_ptr>;

//		//
// 异常	//
//		//

struct exception_info {
	int id;
	exception_info(int _id) :id(_id) {}
};

// 栈溢出异常
class stack_overflow_exception {
	std::shared_ptr<exception_info> _info;
public:
	stack_overflow_exception(int id) : _info(std::shared_ptr<exception_info>(new exception_info(id))) {}

	std::string what() const throw ()
	{
		std::string buf;
		std::stringstream ss;
		ss << _info->id << std::endl;
		ss >> buf;
		std::string info = "stackoverflow exception: blk_id ";
		info += buf + " alloc memory failed.\n";
		return info;
	}
};

// 返回地址信息(默认-1)
struct _return_info {
	size_t return_addr = -1;			// 返回地址
	size_t return_block_id = -1;		// 返回的block id
};

//			//
//  解释器	//
//			//

// 运行时栈帧
struct _StackFrame {
	new_data_list_t local_var_table; // 局部变量表, 根据int做随机访问, 每次有新的标识符就向上增加
	std::vector<data_ptr> stk;		 // 操作数栈
	_return_info ret;
	bool _strong_hold;
};

// 堆栈解释器
using block_ptr = std::shared_ptr<vsblock>;
class vsEvaluator
{
public:
	enum {
		max_stack_size = MAX_STACK_SIZE		// 最大栈帧数, 超过会触发栈溢出异常
	};
private:
	// 友元类
	friend class CommandHelper;
	friend class Command;
	friend class OPERATOR;
	friend class vsVirtualMachine;

	vsVirtualMachine* _vm;								// 所在的虚拟机
	
	vec_command_t* _instruct_ptr;						// 指令数组
	block_ptr _block_ptr;								// block指针, 指向当前的block
	std::vector<_StackFrame> _stk_frame;				// 栈帧

	// std::vector<unsigned int> blk_stk;				// 保存所在语句块数据栈大小恢复值

	const data_ptr null_data = NULL_DATA::null_data;	// null对象

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

protected:
	// 创建并压入栈帧
	void _push_frame() throw(stack_overflow_exception);

	// 弹出局部变量
	void _pop_frame();

public:
	vsEvaluator(vsVirtualMachine* vm)
		:_vm(vm), _stop_val(0), _stop(false), _flag_has_instruct(false) { }

	vsEvaluator(vsVirtualMachine* vm, vec_command_t* ptr)
		:_vm(vm), _instruct_ptr(ptr), _stop(false), _stop_val(0), _flag_has_instruct(true) { }

	~vsEvaluator();

	// 加载block, 在调用call_blk时调用
	void load_block(block_ptr block);

	// 退出block, 会调用pop_frame
	void exit_block();

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

	// 单步执行
	virtual int step();
	
	// 执行全部
	virtual int eval();

	// 停止
	void stop(int stop_val = 0) { _stop = true; _stop_val = stop_val; }

	// 是否停止
	bool isstop() { return _stop; }

	// 获取当前栈
	inline _StackFrame& current_stk_frame() {
		assert(!_stk_frame.empty());
		return _stk_frame.back();
	}

	// 交换顶层两个元素
	inline void reverse_top() {
		assert(!_stk_frame.empty());
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
		assert(!_stk_frame.empty());
		auto& stk = current_stk_frame().stk;

		assert(!stk.empty());
		auto d = stk.back();
		stk.pop_back();
		return d;
	}

	// 返回顶层元素
	inline data_ptr top() {
		assert(!_stk_frame.empty());
		auto& stk = current_stk_frame().stk;

		assert(!stk.empty());
		data_ptr d = stk.back();
		return d;
	}

	// 压入元素
	void push(data_ptr d){
		assert(!_stk_frame.empty());
		auto& stk = current_stk_frame().stk;
		stk.push_back(d);
	}
};

