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

void S_Expr_Compiler::generate_code(const std::vector<Word>& _word_vector, Compile_result& result, Context_helper& helper)
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

				// push 要跳转的地址
				auto& comm = result.refCommandVector(cur_block_id);
				comm.push_back(CommandHelper::getPushOpera(
						data_ptr(new NumData(DataType::BLK_INDEX, block_id))));

				assert(ctool_stk.size() > 1);
				if (!(ctool_stk.rbegin() + 1)->is_def_blk()) {
					// 地址传参
					comm.push_back(COMMAND(PARA_PASS));
					// 生成跳转到block指令
					comm.push_back(COMMAND(CALL_BLK));
				}
			}
			
			// 创建一个block对象, 给予_id
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
			// pop
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
				if (type == WordType::IDENTIFIER_ENABLED && !ctool().is_in_def()){

#if CHECK_Compiler
					std::cout << "如果之前定义过, 并且不在def语句的第一个参数内, 生成解引用代码" << std::endl;
					std::cout << "Context stack POP " << ctx.getName() << std::endl;
					show_cstk("Context stk", ctool()._context_stk);
					show_cstk("temp stk", ctool().tempStk);
#endif
					// 如果 IDENTIFIER_ENABLED 是定义形参
					if (ctool().is_def_paras()) {
						// 为其分配形参下标, 写入临时ctool属性中
						_auto_allc_form_para_index(word);
					}
					else {
						// 寻找局部变量下标, 局部变量会覆盖函数的形参, 使其不可见
						int index = get_alloced_index(word);
						if (index != -1) {
							result.refCommandVector(block_id).push_back(CommandHelper::getPushOpera(data_ptr(new NumData(DataType::ID_INDEX, index))));
							result.refCommandVector(block_id).push_back(COMMAND(NEW_DRF));
						} 
						else {
							// 寻找函数参数下标
							int index_p = get_form_para_alloced_index(word);
							if (index_p != -1) {
								result.refCommandVector(block_id).push_back(CommandHelper::getPushOpera(data_ptr(new NumData(DataType::ID_INDEX, index_p))));
								result.refCommandVector(block_id).push_back(COMMAND(PARA_DRF));
							}
							else {
								// 未定义, 抛出未定义异常(变量必须先声明, 再使用)
								throw undefined_exception(word.getString());
							}
						}
					}
				}
				else {
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