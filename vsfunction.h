#pragma once
#include <memory>
#include <iostream>

#define CHECK_Function true;
#define CHECK_Function_block true;

//
// function
// : 通过evaluator来执行
//

class vsblock;
using blk_ptr = std::shared_ptr<vsblock>;
using vec_command_t = std::vector<Command>;

class vsfunction
{
public:
	vsfunction()
	{
	}

	~vsfunction()
	{
	}
};

//
// vsblock
// : 通过evaluator来执行, 是静态的, 可以通过Data对象来引用
// 

class vsblock
{
	vec_command_t _instruct;		// 指令数组

	size_t _id;						// 分配的地址 _id
	size_t _instruct_length;		// 停止地址(block的地址长度)
public:

	size_t id() const {
		return _id;
	}

	void setInstruct(const vec_command_t& instruct) {
		this->_instruct = instruct;
	}

	inline vec_command_t& instruct() {
		return this->_instruct;
	}

	// 设置结束点
	inline void setEndAddr() {
		_instruct_length = _instruct.size();
	}

	inline size_t instruct_length() {
		return _instruct_length;
	}

	// 确定起始地址
	vsblock(size_t id) :_id(id) {
#if CHECK_Function_block
		std::cout << "Def block startAddr := " << _id << std::endl;
#endif
	}

	virtual ~vsblock() {
	}
};