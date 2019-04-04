#pragma once
#include <memory>
#include <iostream>

#include "Data.h"
#include "vsEvaluator.h"

#define CHECK_Function true;
#define CHECK_Function_block true;

//
// function
// : 通过evaluator来执行
//

class vsblock;
using blk_ptr = std::shared_ptr<vsblock>;
class vsfunction
{
	vec_command_t instruct;		// 指令数组
	blk_ptr _vsblock;
public:
	vsfunction();
	virtual ~vsfunction();
};

//
// vsblock
// : 通过evaluator来执行
// 
class vsblock
{
	vec_command_t _instruct;		// 指令数组

	size_t _id;						// 分配的地址 _id
	size_t _instruct_length;		// 停止地址(block的地址长度)
	size_t _start_addr;				// 起点地址
public:

	void setInstruct(const vec_command_t& instruct) {
		this->_instruct = instruct;
	}

	inline vec_command_t& instruct() {
		return this->_instruct;
	}

	// 设置结束点
	inline void setEndAddr(size_t length) {
		_instruct_length = length;
	}

	inline size_t instruct_length() {
		return _instruct_length;
	}

	// 设置起点地址
	inline void setStartAddr(size_t startAddr) {
		_start_addr = startAddr;
	}

	inline size_t startAddr() {
		return _start_addr;
	}

	// 确定起始地址
	vsblock(size_t id, size_t startAddr) :_id(id), _start_addr(startAddr) {
#if CHECK_Function_block
		std::cout << "Def block startAddr := " << _id << std::endl;
#endif
	}

	virtual ~vsblock() {
#if CHECK_Function_block
		std::cout << "Delete block := " << _id << std::endl;
#endif
	}
};