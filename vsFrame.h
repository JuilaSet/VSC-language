﻿#pragma once

// 返回地址信息(默认-1)
struct _return_info {
	size_t return_addr = -1;			// 返回地址
	size_t return_block_id = -1;		// 返回的block id
};

// 参数信息, 不要存放指针防止潜复制问题
struct _StackFrameParasInfo {
	para_var_container_ptr act_para_container;	// 实参结合表<id -> data_ptr>
	pass_paras_list_t pass_paras_list;	// 实参列表

	// 构造
	_StackFrameParasInfo();

	// 初始化
	void init();
};

// 将要传递给下一帧的信息, 每次传递完成后会初始化, 不要存放指针防止潜复制问题
struct _tempStackFrame {
	std::vector <_StackFrameParasInfo> next_paras_info; // 参数信息栈, 每一次call都push一层

	// 初始化
	void init() {
		next_paras_info.clear();
	}

	// 添加一次信息
	void push_next_paras_info() {
		next_paras_info.emplace_back();
	}

	// 消去信息
	void pop_next_paras_info() {
		next_paras_info.pop_back();
	}

	// 最顶层的(参数)
	_StackFrameParasInfo& backParasInfo() {
		return next_paras_info.back();
	}
};

// 运行时栈帧, 相当于block的一个状态, 不要存放指针防止潜复制问题
struct _StackFrame {
	_tempStackFrame temp_stkframe;	 // 将要传递给下一帧的信息

	_StackFrameParasInfo paras_info;		 // 参数信息
	para_var_container_ptr local_var_table;	 // 以字符串做索引的局部变量表
	std::vector<data_ptr> stk;		 // 操作数栈

	int _paras_count;				 // 实参个数
	_return_info ret;				 // 返回信息
	bool _strong_hold;				 // 当前作用域是否是强作用域

	_StackFrame();

	// 添加一次信息
	void push_next_temp_paras_info() {
		temp_stkframe.push_next_paras_info();
	}

	// 消去信息
	void pop_next_temp_paras_info() {
		temp_stkframe.pop_next_paras_info();
	}

	// 形参结合
	void pass_paras(block_ptr innerblock);

	// 传递信息
	void pass_message(_tempStackFrame& tframe, int paras_count) {
		_paras_count = paras_count;
		paras_info = tframe.backParasInfo();
		// ...
	}

	// 得到当前运行的帧的上下文(保存了外部帧)
	std::shared_ptr<_StackFrame> get_current_function_context();
};

