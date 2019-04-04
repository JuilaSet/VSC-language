#pragma once
#include "vsEvaluator.h"

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

// 虚拟机
class vsVirtualMachine
{
	vec_command_t _instruct;		// 指向当前执行的instruct []][
	unsigned int ipc;				// ip指针
public:
	void run(vsEvaluator& eval);	// 执行虚拟机代码

	vsVirtualMachine() {}
	vsVirtualMachine(vec_command_t instruct) : _instruct(instruct) {}
	~vsVirtualMachine() {};
};

