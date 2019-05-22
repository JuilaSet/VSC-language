#include "pch.h"
#include "Compiler.h"

#if CHECK_Eval_command
void show_comms(const std::vector<Command>& commdVec) {
	std::cout << "c > ";
	for (auto& comm : commdVec) {
		std::cout << "[" << comm.name() << "] ";
	}
	std::cout << std::endl;
}
#endif

#if CHECK_Compiler
void show_cstk(const std::string& tag, std::vector<Context>& stk) {
	std::cout << "+++" << tag << "> ";
	std::for_each(stk.begin(), stk.end(), [&](auto ctx) {
		std::cout << "[" << ctx.getName() << "]" << std::ends;
	});
	std::cout << std::endl;
}
#endif

S_Expr_Compiler::S_Expr_Compiler()
{
}

S_Expr_Compiler::~S_Expr_Compiler()
{
}

void S_Expr_Compiler::generate_code(const std::vector<Word>& _word_vector, 
	Compile_result& result, 
	Context_helper& helper, 
	size_t prc_id)

	throw (Context_error, undefined_exception) {
#if CHECK_Compiler
	std::cout << "\n\nGenerate Code Begin:" << std::endl;
#endif
	_cresult_ptr = &result;
	auto begin = _word_vector.begin();
	auto end = _word_vector.end();
	for (auto it = begin; it != end; ++it) {
#if CHECK_Compiler
		std::cout << "\n" << std::endl;
#endif		
		Word word = *it;
		// 数字和字符串是基本类型, 直接push进入
		WordType type = word.getType();
		std::string context_name = word.serialize();
		Context ctx;
		if (type == WordType::LOCAL_OPEN) {
			// 入栈ctool
			localBegin();
			
			assert(!ctool_stk.empty());
			ctool().blk_op_count = 0;

			// 当前地址 = 是否生成过代码, 没有就是1
			size_t comm_pos = _cur_comm_pos(result);

			// 为新的block分配下标
			size_t block_id = _alloc_block_index();
			
			// 如果已经有block, 就在这一层写入跳转指令(push 跳转的block_id, call_blk)
			if (result.has_block()) {
				assert(!_vsblock_index_vec.empty()); // 这里出错, 说明result的block_map与compiler的_vsblock_index_vec不对应
				auto cur_block_id = _cur_block_id();

				// push 函数对象(保存了要跳转的地址)
				auto& comm = result.refCommandVector(cur_block_id);
				comm.push_back(CommandHelper::getPushOpera(
						data_ptr(new FunctionData(block_id, prc_id))));

				assert(ctool_stk.size() > 1);
				if (!(ctool_stk.rbegin() + 1)->is_def_blk()) {
					comm.push_back(COMMAND(CALL_BLK_BEGIN));
					// 地址传参
					comm.push_back(COMMAND(PARA_PASS));
					// 告知传参个数
					comm.push_back(CommandHelper::getPushOpera(data_ptr(new NumData(0))));
					// 生成跳转到block指令
					comm.push_back(COMMAND(CALL_BLK));
				}
			}
			
			// 创建一个block对象, 给予_id, 设置形参列表
			new_block(result, block_id);

			// block id入栈
			_vsblock_index_vec.push_back(block_id);
			
			// 开启局部变量栈
			result.refCommandVector(block_id).push_back(COMMAND(LOCAL_BEGIN));

#if CHECK_Compiler
			std::cout << "Word = " << word.serialize() << std::endl;
			show_cstk("Context stk", ctool()._context_stk);
			show_cstk("temp stk", ctool().tempStk);
#endif		
		}
		else if (type == WordType::LOCAL_CLOSED) {
#if CHECK_Compiler
			std::cout << "Word = " << word.serialize() << std::endl;
			for (auto& t : ctool_stk) {
				if (!ctool_stk.empty()) {
					show_cstk("Context stk", t._context_stk);
					show_cstk("temp stk", t.tempStk);
				}
				std::cerr << std::endl;
			}
#endif

			auto block_id = _cur_block_id();
#if CHECK_Eval_command
			std::cout << "Block instruct: " << std::endl;
			result.for_each_block([](auto blk) {
				std::cout << "Block " << blk->id() << ':';
				show_comms(blk->instruct());
			});
#endif

			// 退出这层block
			_vsblock_index_vec.pop_back();

			// 恢复工具栈
			localEnd();

			// 关闭局部变量栈
			result.refCommandVector(block_id).push_back(COMMAND(LOCAL_END));
		
			// 如果当前没有外部的block, 程序退出编译
			if (_vsblock_index_vec.empty()) {
				// 确定当前的block中的指令长度
				result.get_block_ref(block_id)->setEndAddr();
				// 如果为空, 结束
				goto END;
			}

			// 确定当前的block中的指令长度
			result.get_block_ref(block_id)->setEndAddr();

#if CHECK_Compiler
			std::cout << "生成上一层block操作符的代码" << std::endl;
			for (auto& t : ctool_stk) {
				if (!ctool_stk.empty()) {
					show_cstk("Context stk", t._context_stk);
					show_cstk("temp stk", t.tempStk);
				}
				std::cerr << std::endl;
			}
#endif

			// block具有原子性: 生成上一层block操作符的代码
			assert(!ctool_stk.empty());

			// 如果之前没有上下文， 就不生成代码
			if (!ctool()._context_stk.empty()) {
				ctx = ctool()._context_stk.back();

				assert(!ctool()._context_stk.empty());
				block_id = _cur_block_id();

				// 如果不是always类型就丢弃这个
				if (ctx.type != Context_Type::ALWAYS) {
					ctool()._context_stk.pop_back();
#if CHECK_Compiler
					std::cout << "如果不是always类型就丢弃这个:" << std::endl;
					if (!ctool_stk.empty()) {
						show_cstk("Context stk", ctool()._context_stk);
						show_cstk("temp stk", ctool().tempStk);
					}
#endif
				}
				auto commands = ctx.getCommandSet(ctool()._context_stk, result.refCommandVector(block_id), word, this);
				for (auto commad : commands) {
#if CHECK_Eval_command
					std::cout << "生成代码:" << commad.name() << std::endl;
#endif
					result.refCommandVector(block_id).push_back(commad);
				}
			}
		}
		else if (type == WordType::STRING_OPEN || type == WordType::STRING_CLOSED) {
#if CHECK_Compiler
			std::cout << "Word = " << word.serialize() << std::endl;
			for (auto& t : ctool_stk) {
				if (!ctool_stk.empty()) {
					show_cstk("Context stk", t._context_stk);
					show_cstk("temp stk", t.tempStk);
				}
				std::cerr << std::endl;
			}
#endif
			// 什么都不做
			assert(!ctool_stk.empty());
		}
		else if (type == WordType::SEPARATOR) {
#if CHECK_Compiler
		std::cout << "Word = " << word.serialize() << std::endl;
		for (auto& t : ctool_stk) {
			if (!ctool_stk.empty()) {
				show_cstk("Context stk", t._context_stk);
				show_cstk("temp stk", t.tempStk);
			}
			std::cerr << std::endl;
		}
#endif
			// 什么都不做
			assert(!ctool_stk.empty());
		}
		else if (type == WordType::CONTEXT_CLOSED) {
			assert(!ctool_stk.empty());
			// 遇到上下文结束符";"
#if CHECK_Compiler
			std::cout << "Word = " << word.serialize() << std::endl;
#endif	
			size_t block_id = _cur_block_id();
			// 如果之前没有上下文就忽略
			if (!ctool()._context_stk.empty()) {

				// pop到END处为止, 忽略之后所有的参数
				while (ctx.type != Context_Type::END) {
					// pop
					ctx = ctool()._context_stk.back();
					ctool()._context_stk.pop_back();
#if CHECK_Compiler
					std::cout << "Context stack POP = " << ctx.getName() << std::endl;
					for (auto& t : ctool_stk) {
						if (!ctool_stk.empty()) {
							show_cstk("Context stk", t._context_stk);
							show_cstk("temp stk", t.tempStk);
						}
						std::cerr << std::endl;
					}
#endif
				}

				// 生成代码
				auto commands = ctx.getCommandSet(ctool()._context_stk, result.refCommandVector(block_id), word, this);
				for (auto commad : commands) {
					result.refCommandVector(block_id).push_back(commad);
				}

				// 如果之前保存了外部op, 就生成外部op的COMMAND
				if (!ctool().tempStk.empty()) {
					ctx = ctool().tempStk.back();
					ctool().tempStk.pop_back();
#if CHECK_Compiler
					std::cerr << "TEMP STK POP = " << ctx.getName() << std::endl;
					show_cstk("Context stk", ctool()._context_stk);
					show_cstk("temp stk", ctool().tempStk);
#endif		
					// 生成代码
					auto commands = ctx.getCommandSet(ctool()._context_stk, result.refCommandVector(block_id), word, this);
					for (auto commad : commands) {
						result.refCommandVector(block_id).push_back(commad);
					}
				}

				// 判断是否生成SHRINK指令
				assert(!ctool_stk.empty());
				if ((ctool().blk_op_count -= 1) == 0) {
					result.refCommandVector(block_id).push_back(COMMAND(SHRINK));
				}
			}
		}
		else if (type == WordType::NUMBER || type == WordType::STRING || type == WordType::IDENTIFIER_ENABLED) {	// 新增, 对基本类型的操作
#if CHECK_Compiler
			std::cout << "Word = " << word.serialize() << std::endl;
#endif		
			auto block_id = _cur_block_id();

			if (ctx.type != Context_Type::END) {		// 忽略END上下文
#if CHECK_Compiler
				std::cout << "Context stack POP = " << ctx.getName() << std::endl;
				std::cout << "COMMAND: PUSH " << context_name << std::endl;
				for (auto& t : ctool_stk) {
					if (!ctool_stk.empty()) {
						show_cstk("Context stk", t._context_stk);
						show_cstk("temp stk", t.tempStk);
					}
					std::cerr << std::endl;
				}
#endif
				// pop	
				if (type == WordType::IDENTIFIER_ENABLED && !ctool().is_in_def()){

#if CHECK_Compiler
					std::cout << "如果之前定义过, 并且不在def语句的第一个参数内, 生成解引用代码" << std::endl;
					std::cout << "Context stack POP " << ctx.getName() << std::endl;
					show_cstk("Context stk", ctool()._context_stk);
					show_cstk("temp stk", ctool().tempStk);
#endif
					// 如果 IDENTIFIER_ENABLED 是定义形参
					if (ctool().is_def_paras()) {
						// 设置形参, 写入临时ctool属性中
						insert_form_para(word);
					}
					else {
						// 寻找局部变量下标, 局部变量会覆盖函数的形参, 使其不可见
						if (has_local(word)) {
							result.refCommandVector(block_id).push_back(CommandHelper::getPushOpera(data_ptr(new IndexData(DataType::ID_INDEX, word.getString()))));
							result.refCommandVector(block_id).push_back(COMMAND(NEW_DRF));
						} 
						else {
							// 寻找函数参数下标
							if (has_para(word)) {
								result.refCommandVector(block_id).push_back(CommandHelper::getPushOpera(data_ptr(new IndexData(DataType::PARA_INDEX, word.getString()))));
								result.refCommandVector(block_id).push_back(COMMAND(PARA_DRF));
							}
							else {
								// 未定义, 抛出未定义异常(变量必须先声明, 再使用)
								throw undefined_exception(word.getString());
							}
						}
					}
				}
				else if (type == WordType::IDENTIFIER_ENABLED){
					// 是标识符且在定义语句中(def assign cp)
					if (has_para(word)) {
						// 对形参赋值
						result.refCommandVector(block_id).push_back(CommandHelper::getPushOpera(data_ptr(new IndexData(DataType::PARA_INDEX, word.getString()))));
					}
					else {
						// 默认是局部变量
						result.refCommandVector(block_id).push_back(CommandHelper::getPushOpera(data_ptr(new IndexData(DataType::ID_INDEX, word.getString()))));
					}
				}
				else {
					// 其他非标识符
					result.refCommandVector(block_id).push_back(CommandHelper::getPushOpera(word.getData()));	// push 立即数
				}
				
				// 生成操作符的代码
				assert(!ctool_stk.empty());

				// 如果之前没有上下文， 就不生成代码
				if (!ctool()._context_stk.empty()) {
					ctx = ctool()._context_stk.back();
					// 如果不是always类型就丢弃这个
					if (ctx.type != Context_Type::ALWAYS) ctool()._context_stk.pop_back();
					auto commands = ctx.getCommandSet(ctool()._context_stk, result.refCommandVector(block_id), word, this);
					for (auto commad : commands) {
						result.refCommandVector(block_id).push_back(commad);
					}
				}
			}
		}
		else if (type == WordType::IDENTIFIER_SPEC) {
			auto block_id = _cur_block_id();
			if (ctx.type != Context_Type::END) {		// 忽略END上下文
#if CHECK_Compiler
				std::cout << "Word = " << word.serialize() << std::endl;
#endif
				assert(!ctool_stk.empty());
				// 直接生成代码
				ctx = helper.get_context(context_name);
				assert(!ctool()._context_stk.empty());
				auto commands = ctx.getCommandSet(ctool()._context_stk, result.refCommandVector(block_id), word, this);	// 在这里push新的上下文
#if CHECK_Compiler
				for (auto& t : ctool_stk) {
					if (!ctool_stk.empty()) {
						show_cstk("Context stk", t._context_stk);
						show_cstk("temp stk", t.tempStk);
					}
					std::cerr << std::endl;
				}
#endif		
				for (auto commad : commands) {
					result.refCommandVector(block_id).push_back(commad);
				}

				// 生成前一个上下文的代码
				ctx = ctool()._context_stk.back();
				if (ctx.type != Context_Type::ALWAYS) ctool()._context_stk.pop_back();
				auto commands_last = ctx.getCommandSet(ctool()._context_stk, result.refCommandVector(block_id), word, this);
				for (auto commad : commands_last) {
					result.refCommandVector(block_id).push_back(commad);
				}
			}
		}
		else if (type == WordType::OPERATOR_WORD || type == WordType::CONTROLLER) {
#if CHECK_Compiler
			std::cout << "Word = " << word.serialize() << std::endl;
#endif
			auto block_id = _cur_block_id();
			assert(!ctool_stk.empty());
			// pop to tempstk
			if (!ctool()._context_stk.empty()) {	// void -> atomic
				ctx = ctool()._context_stk.back();
				if (ctx.type != Context_Type::END) {		// 忽略END类型
					ctool().tempStk.push_back(ctx);
					if (ctx.type != Context_Type::ALWAYS) ctool()._context_stk.pop_back();
#if CHECK_Compiler
					std::cout << "Context = " << ctx.getName() << " stack POP into Temp" << std::endl;
					show_cstk("Context stk", ctool()._context_stk);
					show_cstk("temp stk", ctool().tempStk);
#endif
				}
			}

			// 操作符栈顶+1
			assert(!ctool_stk.empty());
			ctool().blk_op_count += 1;

			// 生成代码
			ctx = helper.get_context(context_name);
			auto commands = ctx.getCommandSet(ctool()._context_stk, result.refCommandVector(block_id), word, this);	// 在这里push新的上下文
#if CHECK_Compiler
			std::cout << "Get new Context stack size" << std::endl;
			for (auto& t : ctool_stk) {
				if (!ctool_stk.empty()) {
					show_cstk("Context stk", t._context_stk);
					show_cstk("temp stk", t.tempStk);
				}
				std::cerr << std::endl;
			}
#endif		
			for (auto commad : commands) {
				result.refCommandVector(block_id).push_back(commad);
			}
		}


#if CHECK_Eval_command
		auto block_id = _cur_block_id();
		show_comms(result.refCommandVector(block_id));
#endif

	}
END:
#if CHECK_Eval_command
	// 显示最终的结果
	std::cout << std::endl << "Finally :" << std::endl;
	result.for_each_block([](auto blk) {
		std::cout << "Block " << blk->id() << ':';
		show_comms(blk->instruct());
		std::cout << std::endl;
	});
#endif
	_cresult_ptr = nullptr;
	return;
}

// 结点定义符号
const std::string Node_Compiler::NODE_DEF = "*";

// 结点会话符号
const std::string Node_Compiler::NODE_SESS = "#";

// 定义花括号
const std::string Node_Compiler::NODE_DEF_OPEN = "{";
const std::string Node_Compiler::NODE_DEF_CLOSED = "}";

// 连接符号
const std::string Node_Compiler::NODE_LIINK = "->";

// 结束符号
const std::string Node_Compiler::NODE_END = ";";

// 冒号
const std::string Node_Compiler::NODE_ADM = ":";

// 使用函数递归的方式分析记号流
void Node_Compiler::top() throw (undefined_exception) {
	assert(_word_vec.size() > _pos);
	auto& w = _getWord(_pos);
	auto str = w.getString();
	auto type = w.getType();
	
	// 是多个结点定义符号
	while (str == NODE_DEF) {
		_pos++; // 跳过*号
		nodeBlock();

		// 下一个单词
		assert(_word_vec.size() > _pos);
		w = _getWord(_pos);
		str = w.getString();
		type = w.getType();
	}
	
	// 可以是一个结点组织语句
	while(type == WordType::IDENTIFIER_ENABLED) {
		nodeLinkExpr();
		w = _getWord(_pos);
		str = w.getString();
		type = w.getType();
	}
	_pos++;
	
	// 可以是一个结点调用语句
	if (str == NODE_SESS) {
		w = _getWord(_pos++);
		auto name = w.getString();
		if (_contain(name))
			_sess_point = _find_node(name)->getId();
		else
			throw undefined_exception("未定义结点" + name);
	}
}

// 结点声明语句
void Node_Compiler::nodeBlock() throw (undefined_exception) {
	assert(_word_vec.size() > _pos);
	auto& w = _getWord(_pos);
	auto name = w.getString();
	// 创建这个结点, 名称为node定义时的名称
	if (_contain(name)) {
		throw undefined_exception("重定义" + name + "结点!");
	}
	auto node = LeafNode::create(name);
	// 进入结点定义语句
	_pos++; // 跳过结点名
	nodeDef(node);
}

// 结点声明语句 := * 名 { 结点定义语句... }
void Node_Compiler::nodeDef(std::shared_ptr<LeafNode> node) {
	assert(_word_vec.size() > _pos);
	auto& w = _getWord(_pos);
	auto str = w.getString();
	auto type = w.getType();
	// 是大括号, 进入定义语句
	if (str == NODE_DEF_OPEN) {
		_pos++; // 跳过{括号
		while (true) {
			// 开始
			defExpr(node);

			// 如果是结束语句就退出
			if (_word_vec.size() <= _pos) {
				std::cerr << "超过边界:" << _pos << std::endl;
				assert(false);
			}
			auto& w = _getWord(_pos);
			auto str = w.getString();
			auto type = w.getType();
			if (str == NODE_DEF_CLOSED) {
				_pos++; // 跳过}括号
				break;
			}
		}
	}
}

// 结点定义语句 := < * >  属性 : 值 ;
void Node_Compiler::defExpr(std::shared_ptr<LeafNode> node) {
	assert(_word_vec.size() > _pos);
	auto& w = _getWord(_pos);
	auto str = w.getString();
	auto type = w.getType();

	// 是 * 符号
	if (str == NODE_DEF) {
		_pos++; // 跳过*号

		// 下一个单词
		assert(_word_vec.size() > _pos);
		w = _getWord(_pos);
		str = w.getString();
		type = w.getType();
	}
	else {
		assert(false); //说明语法分析出错
	}

	// 是属性
	if (type == WordType::IDENTIFIER_ENABLED || type == WordType::OPERATOR_WORD) {
		assert(_word_vec.size() > _pos);
		std::string attr = _getWord(_pos).getString();
		_pos++; // 跳过属性名

		// 冒号
		assert(_word_vec.size() > _pos);
		if (_getWord(_pos).getString() == NODE_ADM) {
			_pos++; // 跳过冒号

			// 之后会跟上值, 查看类型, 一直到分号为止
			assert(_word_vec.size() > _pos);
			auto value_word = _getWord(_pos);
			if (value_word.getType() == WordType::NUMBER) {
				node->pushData(attr, data_ptr(value_word.getData()));
				// 直到最后一个分号为止
				assert(_word_vec.size() > _pos);
				while (_getWord(_pos).getString() != NODE_END) // 跳过分号
					_pos++;
			}
			// 集合
			else if (value_word.getType() == WordType::IDENTIFIER_ENABLED) {
				std::shared_ptr<vsVector> vec(new vsVector);
				while (value_word.getType() != WordType::CONTEXT_CLOSED) { // 到分号为止, 将前面所有字符串加入集合
					assert(_word_vec.size() > _pos);
					vec->push_data(value_word.getData());
					// 下一个单词
					value_word = _getWord(++_pos);
				}
				node->pushData(attr, vec);
			}
			// 字符串
			else {
				assert(_word_vec.size() > _pos);
				std::string value;
				// 直到最后一个分号为止, 过滤字符串标记
				for (;; _pos++) {
					assert(_word_vec.size() > _pos);
					auto& w = _getWord(_pos);
					auto temp_type = w.getType();
					if (temp_type == WordType::STRING) {
						value += w.getString();
					}
					else if (temp_type == WordType::CONTEXT_CLOSED) {
						break;
					}
				}
				node->pushData(attr, data_ptr(new StringData(value)));
			}
		}
		// 没有冒号, 直接分号结束
	}
	// 跳过分号
	_pos++;
	// 结束定义, 加入集合
	_nodes.insert_or_assign(node->getName(), node);
}

// 结点组织语句 := 名 -> 名 { -> 名 }
void Node_Compiler::nodeLinkExpr()throw(undefined_exception) {
	// 名
	auto& w = _getWord(_pos++);
	auto name = w.getString();
	if (!_contain(name)) {
		// 抛出未定义异常
		throw undefined_exception("未定义" + name + "结点!");
	}
	// 第一次时是head
	if (gbuilder.is_empty()) {
		auto node = _find_node(name);

		node->setID(_count++);
		gbuilder.head(node);
	}

	// 下一次表示跳转到这个结点
	else {
		// 如果没有被设置过id, 才进行设置和添加, 否则视为跳转
		auto node = _find_node(name);
		if (node->getId() == -1) {
			node->setID(_count);
			gbuilder.addNode(_count++, node);
		}
		else {
			// 跳转
			gbuilder.gotoNode(_find_node(name)->getId());
		}
	}

	// 后续"->名"
	while (true) {
		// { -> 名 }
		auto& w = _getWord(_pos++);// 跳过 "->"或";"
		auto link = w.getString();
		auto type = w.getType();
		// ->号
		if (link == NODE_LIINK) {
			auto& w = _getWord(_pos++);	// 跳过 node name
			auto node_name = w.getString();
			if (!_contain(node_name)) {
				// 抛出未定义异常
				throw undefined_exception("未定义" + node_name + "结点!");
			}
			auto node = _find_node(node_name);
			// 如果没有被设置过id, 才进行设置和添加, 否则视为连接 
			if (node->getId() == -1) {
				node->setID(_count);
				gbuilder.addNode(_count++, node);
			}
			else {
				// 连接结点
				gbuilder.linkToNode(node->getId());
			}
		}
		// ;号
		else if (type == WordType::CONTEXT_CLOSED) {
			break;
		}
	}
}
