#pragma once

// 
class Command;
class CommandHelper;
class vsVirtualMachine;
class vsblock_static;

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

//			//
//  解释器	//
//			//

// 堆栈解释器
using block_ptr = std::shared_ptr<vsblock_static>;
class vsEvaluator
{
public:
	enum {
		max_stack_size = MAX_STACK_SIZE		// 最大栈帧数, 超过会触发栈溢出异常
	};
protected:
	// 友元类
	friend class CommandHelper;
	friend class Command;
	friend class OPERATOR;
	friend class vsVirtualMachine;

	vsVirtualMachine* _vm;								// 所在的虚拟机
	
	vec_command_t* _instruct_ptr;						// 指令数组
	block_ptr _block_ptr;								// block指针, 指向当前的block, block在vm中固定
	std::vector<RunTimeStackFrame_ptr> _stk_frame;		// 栈帧指针

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

	// 根据index获取data对象
	data_ptr _find_data(std::string index, local_var_container_ptr& table_ret);

	// 根据index获取para实参对象 
	data_ptr _find_para(std::string index, para_var_container_ptr& act_paras_table_ret);

	// 创建并压入栈帧
	void _push_and_setting_frame(block_ptr retblock, block_ptr curblock, int paras_count) throw(stack_overflow_exception);

	// 弹出局部变量
	void _pop_frame();

protected:
	// 保存返回值
	data_ptr ret_data = NULL_DATA::null_data;

	// 外部数据
	vsIdentifierContainer _extern_datas;

	// 保存程序id, 会向在vm中对应的程序运行
	vsTool::id_t _process_id;
public:

	// 获取外部数据(如果没有找到就返回空对象)
	data_ptr _get_extern_data(std::string index);

	// 设置外部数据(会覆盖原有数据)
	void _add_extern_data(std::string index, data_ptr data);

	// 设置返回值
	void _set_return_data(data_ptr ret_data);

	// 获取返回值
	data_ptr _get_ret_data() {
		return ret_data;
	}

public:
	vsEvaluator(vsVirtualMachine* vm, vsTool::id_t process_id)
		:_vm(vm), _stop_val(0), _stop(false), _flag_has_instruct(false), _process_id(process_id){ }

	vsEvaluator(vsVirtualMachine* vm, vec_command_t* ptr)
		:_vm(vm), _instruct_ptr(ptr), _stop(false), _stop_val(0), _flag_has_instruct(true) { }

	~vsEvaluator();

	// 是否是最外层
	bool isGlobal() {
		return (_stk_frame.size() == 0);
	}

	// 加载block, 在调用call_blk时调用
	void load_block(int enter_point, int paras_count);

	// 退出block, 会调用pop_frame
	void exit_block();

	// 清空运行时栈
	void clear_frame() {
		this->_stk_frame.clear();
	}

	// 初始化解释器
	void init() {
		_stop = false;
		ipc = 0;
		// 清空栈帧
		_stk_frame.clear();
	}		

	// new def
	void new_regist_identity(std::string index, data_ptr d);

	// new assign
	bool new_set_data(std::string index, data_ptr d, bool isCopyMode = false);
	
	// new drf
	data_ptr new_get_data(std::string index);

	// para pass(传递参数给函数)
	void para_pass_data(data_ptr data);

	// para assign(给参数赋值)
	bool para_assign_data(std::string index, data_ptr data, bool isCopyMode = false);

	// para drf(对参数解引用)
	data_ptr para_get_data(std::string index);

	// 单步执行
	virtual int step();
	
	// 执行全部
	virtual int eval();

	// 停止
	void stop(int stop_val = 0) { _stop = true; _stop_val = stop_val; }

	// 是否停止
	bool isstop() { return _stop; }

	// 获取当前栈
	inline RunTimeStackFrame_ptr current_stk_frame() {
		assert(!_stk_frame.empty());
		return _stk_frame.back();
	}

	// 交换顶层两个元素
	inline void reverse_top() {
		assert(!_stk_frame.empty());
		auto& stk = current_stk_frame()->stk;

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
		auto& stk = current_stk_frame()->stk;

		assert(!stk.empty());
		auto d = stk.back();
		stk.pop_back();
		return d;
	}

	// 返回顶层元素
	inline data_ptr top() {
		assert(!_stk_frame.empty());
		auto& stk = current_stk_frame()->stk;

		assert(!stk.empty());
		data_ptr d = stk.back();
		return d;
	}

	// 压入元素
	void push(data_ptr d){
		// 说明在没有push 栈帧的时候就尝试往其中放入数据
		assert(!_stk_frame.empty());
		auto& stk = current_stk_frame()->stk;
		stk.push_back(d);
	}

	// 返回虚拟机指针
	vsVirtualMachine* getVM() {
		return _vm;
	}
};

