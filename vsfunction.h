#pragma once
#include <memory>
#include <iostream>

#define CHECK_Function true;
#define CHECK_Function_block true;

//
// vsblock
// : 通过evaluator来执行, 是静态的, 可以通过Data对象来引用
// 

class vsblock
{
protected:
	vec_command_t _instruct;		// 指令数组

	size_t _id;						// 分配的地址 _id
	size_t _instruct_length;		// 停止地址(block的地址长度)

	bool _strong_hold;				// 是否是强作用域
	std::vector<data_ptr> _paras_vec;	// 指令参数位置 -> 传入数据
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

	inline bool strong_hold() { return _strong_hold; }
	
	//
	// 参数相关
	//

	virtual data_ptr getParas(size_t) {
		return NULL_DATA::null_data;
	}

	// 确定id
	vsblock(size_t id, bool strong_hold = false) :_id(id), _strong_hold(strong_hold) {
#if CHECK_Function_block
		std::cout << "Def block startAddr := " << _id << std::endl;
#endif
	}

	virtual ~vsblock() { }
};

using block_ptr = std::shared_ptr<vsblock>;

//
// function
// : 通过evaluator来执行
//

using blk_ptr = std::shared_ptr<vsblock>;
using vec_command_t = std::vector<Command>;

class vsfunction : public vsblock
{

public:
	vsfunction(size_t _id): vsblock(_id)
	{
	}

	~vsfunction()
	{
	}
};