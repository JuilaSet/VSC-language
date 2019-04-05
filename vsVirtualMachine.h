#pragma once

#define COPY_TO_BIT_SET(bitarr, begin, non_char_data, size) \
		char* cptr = (char*)(&(non_char_data));			\
		std::memcpy(bitarr + (begin), cptr, (size));	\
//

// 存放指令的结构体
class op_code_bit_set {
	// 描述数据 
	size_t _data_size;

	// 数据
	bool _is_string;
	int32_t _opcode;
	void* _data;	// 数字下为int

public:
	// 头部的大小
	enum { head_length = 1 };

	op_code_bit_set(OPCODE opcode, int i)
		:_opcode((int)opcode), _is_string(false), _data_size(sizeof(i)), _data((void*)i) { }

	op_code_bit_set(OPCODE opcode, const char str[], int str_size)
		:_opcode((int)opcode), _is_string(true), _data_size(str_size), _data(new char[str_size]) {
		std::memcpy(_data, str, str_size);
	}

	// 转为位图
	std::shared_ptr<char*> to_bit_set() {
		size_t opcode_size = sizeof(_opcode);
		size_t data_size = sizeof(_data);
		size_t bit_set_size = head_length + opcode_size + _data_size + 1;
		auto bitarr = std::make_shared<char*>(new char[bit_set_size]);

		// 第一位表示是否是字符串
		(*bitarr)[0] = _is_string;
		// 将opcode以小端形式存放到arr中
		COPY_TO_BIT_SET(*bitarr, head_length, _opcode, opcode_size);
		if (_is_string) {
			// 将字符串存放到bitarr中
			std::memcpy(*bitarr + opcode_size + head_length, (char*)_data, _data_size);
		}
		else {
			// 将数字存放到字bitarr中
			COPY_TO_BIT_SET(*bitarr, opcode_size + head_length, _data, data_size);
		}
		(*bitarr)[bit_set_size - 1] = '\0';
		return bitarr;
	}

	// 位图写入时的大小
	size_t write_size() {
		return head_length + sizeof(int) + _data_size + 1;
	}

	// data的大小
	size_t data_size() {
		return _data_size;
	}

	// 位图的大小(opcode + _data大小)
	size_t size() {
		return _data_size + sizeof(int);
	}

#if CHECK_Eval
	void display() {
		auto arr = to_bit_set();
		// 逐字节显示
		auto s = write_size();
		for (size_t i = 0; i < s; ++i) {
			std::cout << std::hex << "[" << (int)(*arr)[i] << "]";
		}
		std::cout << std::endl;
	}
#endif

	~op_code_bit_set() {
		if (_is_string) delete[] _data;
	}
};

///
/// 异常类
///

// 虚拟机没有准备完毕
struct vsVm_not_inited_exception : public std::exception 
{
private:
	int _vsvm_id;
public:
	vsVm_not_inited_exception(size_t vsvmid) : _vsvm_id(vsvmid) {}
	const char * what() const throw ()
	{
		std::stringstream ss;
		ss << "id = " << _vsvm_id << " vsvm not ready Exception";
		std::string info;
		ss >> info;
		return info.c_str();
	}
};

// 虚拟机
class vsVirtualMachine
{
	size_t _id;							// 虚拟机的id


	///
	/// 属性
	///

	std::map<size_t, vsblock> _sb_map;	// block表 (id -> vsblock)
	std::shared_ptr<vsEvaluator> _eval_main; // 表示主线程的解释器
	size_t _enter_point;				// 入口点

	///
	/// 标志flag
	///

	bool _inited;				// 是否初始化完成, 即完成加载工作(block表加载, 入口点设置)
	int _eval_ret_value;		// 解释器的退出值
public:
	void run();	// 执行虚拟机代码
	
	/// 辅助函数
	///

	// 获取block
	vsblock& get_block_ref(size_t block_index) {
		assert(!_sb_map.empty());
		auto it = _sb_map.find(block_index);
		assert(it != _sb_map.end());
		auto& block = it->second;
		return block;
	}

	// 获取退出值
	int get_eval_ret_value() {
		return _eval_ret_value;
	}

	///
	/// 虚拟机函数
	///

	// 初始化并设置block表
	void init(Compile_result cresult, size_t enter_point){
		_sb_map = cresult.get_sb_map();
		_enter_point = enter_point;
		_inited = true;
	}

	// 初始化时，不指定属性
	vsVirtualMachine(size_t id) 
		: _id(id), _inited(false) {}

	// 直接通过结果对象来初始化
	vsVirtualMachine(size_t id, Compile_result cresult, size_t enter_point)
		: _id(id), _sb_map(cresult.get_sb_map()), _enter_point(enter_point), _inited(true){}
	
	// 通过block表来初始化
	vsVirtualMachine(size_t id, std::map<size_t, vsblock> sb_map, size_t enter_point)
		: _id(id), _sb_map(sb_map), _enter_point(enter_point), _inited(true){}

	~vsVirtualMachine() {};
};