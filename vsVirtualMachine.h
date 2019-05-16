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

// 虚拟机没有准备完毕异常
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
	friend class vsEvaluator;
protected:
	size_t _id;									// 虚拟机的id

	////
	//// 属性
	////

	std::map <vsTool::id_t, std::map<size_t, block_ptr>> _sb_map;	// block表 (id -> vsblock), 只读s

	// 初始化计算图图
	vsThread::taskGraphic_ptr<std::string, data_ptr> _taskGraphic;

	////
	//// 标志flag
	////

	bool _inited;							// 是否初始化完成, 即完成加载工作(block表加载, 入口点设置)
	std::atomic<int> _eval_ret_value;		// 解释器的退出值

public:
	// 执行虚拟机代码, 返回一个int表示状态
	data_ptr run(vsTool::id_t id, std::string aim);
	
	/// 辅助函数
	///

	// 获取block
	block_ptr get_block_ref(size_t block_index, vsTool::id_t process_id) const {
		assert(!_sb_map.empty());
		auto mapit = _sb_map.find(process_id);
		assert(mapit != _sb_map.end());
		auto map = mapit->second;
		assert(!map.empty());
		auto it = map.find(block_index);
//		assert(it != map.end());
		if (it == map.end()) {
			printf("在proc: %d中", process_id);
			printf("没有找到这个block: %d\n", block_index);
			printf("block数量: %d\n", map.size());
			for (auto b : map) {
				std::cout << "Blockid: " << b.first << std::endl;
			}
			assert(false);
		}
		block_ptr block = it->second;
		return block;
	}

	// 获取退出值(非线程安全的)
	int get_return_ref() const {
		return _eval_ret_value;
	}

	///
	/// 虚拟机函数
	///

	// 绑定对应的程序
	std::shared_ptr<vsEvaluator> make_eval(vsTool::id_t process_id) {
		// 创建一个eval解释器
		auto eval_ptr = std::make_shared<vsEvaluator>(vsEvaluator(this, process_id));
		return eval_ptr;
	}

	// 设置图
	void set_graphic(vsThread::taskGraphic_ptr<std::string, data_ptr> g) {
		_taskGraphic = g;
		_inited = true;
	}

	// 设置程序(程序结果, 程序id)
	void add_process(Compile_result cresult, vsTool::id_t processs_id) {
		auto map = cresult.get_sb_map();
		_sb_map.insert(std::make_pair(processs_id, map));
	}

	// 清空线程池和程序池
	void init_pools() {
		_taskGraphic->init();
		_sb_map.clear();
	}

	// 初始化时，不指定属性
	vsVirtualMachine(size_t id)
		: _id(id), _inited(false) {}
};