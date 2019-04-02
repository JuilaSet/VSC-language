#include "pch.h"
#include "Compiler.h"

S_Expr_Compiler::S_Expr_Compiler()
{
}

S_Expr_Compiler::~S_Expr_Compiler()
{
}

void show_cstk(const std::string& tag, std::vector<Context>& stk) {
	std::cout << "+++" << tag << "> ";
	std::for_each(stk.begin(), stk.end(), [&](auto ctx) {
		std::cout << "[" << ctx.getName() << "]" << std::ends;
	});
	std::cout << std::endl;
}

void S_Expr_Compiler::generate_code(const std::vector<Word>& _word_vector, std::vector<Command>& commdVec, Context_helper& helper) 
	throw (Context_error) {
#if CHECK_Compiler
	std::cout << "\n\nGenerate Code Begin:" << std::endl;
#endif
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
		if (type == WordType::LOCAL_OPEN) {	// ctool().blk_op_count入栈0
			localBegin();
			ctool_stk.push_back(_compiler_tool());

			assert(!ctool_stk.empty());
			ctool().blk_op_count = 0;
			// 开启局部变量栈
			commdVec.push_back(Command(OPERATOR::LOCAL_BEGIN));
#if CHECK_Compiler
			std::cout << "Word = " << word.serialize() << std::endl;
			show_cstk("Context stk", ctool()._context_stk);
			show_cstk("temp stk", ctool().tempStk);
#endif		
		}
		else if (type == WordType::LOCAL_CLOSED) {
#if CHECK_Compiler
			std::cout << "Word = " << word.serialize() << std::endl;
			show_cstk("Context stk", ctool()._context_stk);
			show_cstk("temp stk", ctool().tempStk);
#endif
			// 关闭局部变量栈
			commdVec.push_back(Command(OPERATOR::LOCAL_END));

			// 当做上下文结束符";"处理		
#if CHECK_Compiler
			std::cout << "CONTEXT_CLOSED" << std::endl;
#endif	
			// pop到END处为止, 忽略之后所有的参数
			while (ctx.type != Context_Type::END) {
				if (ctool_stk.empty()) {
					// 如果为空, 结束
					goto END;
				}
				assert(!ctool_stk.empty());
				if (!ctool()._context_stk.empty()) {
					ctx = ctool()._context_stk.back();
					ctool()._context_stk.pop_back();
#if CHECK_Compiler
					std::cout << "Context stack POP = " << ctx.getName() << std::endl;
					show_cstk("Context stk", ctool()._context_stk);
					show_cstk("temp stk", ctool().tempStk);
#endif
				}
				else{
					// 恢复工具栈
					ctool_stk.pop_back();
				}
			}

			// 生成代码(这里的上下文一定上面while中找到的END类型)
			assert(!ctool_stk.empty());
			auto commands = ctx.getCommandSet(ctool()._context_stk, commdVec, word, this);
			for (auto commad : commands) {
				commdVec.push_back(commad);
			}

			// 如果之前保存了外层上下文, 就生成外部的COMMAND
			assert(!ctool_stk.empty());
			if (!ctool().tempStk.empty()) {
				Context ctx_t = ctool().tempStk.back();
				ctool().tempStk.pop_back();
#if CHECK_Compiler
				std::cout << __LINE__ << " TEMP stack POP = " << ctx_t.getName() << std::endl;
				show_cstk("Context stk", ctool()._context_stk);
				show_cstk("temp stk", ctool().tempStk);
#endif
				// 生成代码
				auto commands = ctx_t.getCommandSet(ctool()._context_stk, commdVec, word, this);
				for (auto commad : commands) {
					commdVec.push_back(commad);
				}
			}

			// 判断是否生成SHRINK指令
			assert(!ctool_stk.empty());
			if ((ctool().blk_op_count -= 1) == 0) {
				commdVec.push_back(Command(OPERATOR::SHRINK));
			}
			localEnd();
		}
		else if (type == WordType::STRING_OPEN || type == WordType::STRING_CLOSED) {
#if CHECK_Compiler
			std::cout << "Word = " << word.serialize() << std::endl;
			show_cstk("Context stk", ctool()._context_stk);
			show_cstk("temp stk", ctool().tempStk);
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
			// pop到END处为止, 忽略之后所有的参数
			while (ctx.type != Context_Type::END) {
				// pop
				assert(!ctool()._context_stk.empty());
				ctx = ctool()._context_stk.back();
				ctool()._context_stk.pop_back();
#if CHECK_Compiler
				std::cout << "Context stack POP = " << ctx.getName() << std::endl;
				show_cstk("Context stk", ctool()._context_stk);
				show_cstk("temp stk", ctool().tempStk);
#endif
			}

			// 生成代码
			auto commands = ctx.getCommandSet(ctool()._context_stk, commdVec, word, this);
			for (auto commad : commands) {
				commdVec.push_back(commad);
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
				auto commands = ctx.getCommandSet(ctool()._context_stk, commdVec, word, this);
				for (auto commad : commands) {
					commdVec.push_back(commad);
				}
			}

			// 判断是否生成SHRINK指令
			assert(!ctool_stk.empty());
			if ((ctool().blk_op_count -= 1) == 0) {
				commdVec.push_back(Command(OPERATOR::SHRINK));
			}
		}
		else if (type == WordType::NUMBER || type == WordType::STRING || type == WordType::IDENTIFIER_ENABLED) {	// 新增, 对基本类型的操作
#if CHECK_Compiler
			std::cout << "Word = " << word.serialize() << std::endl;
#endif		
			assert(!ctool_stk.empty());
			ctx = ctool()._context_stk.back();
			if (ctx.type != Context_Type::END) {		// 忽略END上下文
			// pop
#if CHECK_Compiler
				std::cout << "Context stack POP = " << ctx.getName() << std::endl;
				std::cout << "COMMAND: PUSH " << context_name << std::endl;
				show_cstk("Context stk", ctool()._context_stk);
				show_cstk("temp stk", ctool().tempStk);
#endif
				if (type == WordType::IDENTIFIER_ENABLED && !ctool().is_in_def()) {
					// 如果不在def或assign语句的第一个参数内, 生成解引用代码(即使是事先没有定义)
					// && isType(word, WordType::IDENTIFIER)
					ctx = ctool()._context_stk.back();
#if CHECK_Compiler
					std::cout << "如果已经定义了, 并且不在def语句的第一个参数内, 生成解引用代码" << std::endl;
					std::cout << "Context stack POP " << ctx.getName() << std::endl;
					show_cstk("Context stk", ctool()._context_stk);
					show_cstk("temp stk", ctool().tempStk);
#endif
					// pop
					// 生成解引用代码
					commdVec.push_back(CommandHelper::getPushOpera(word.getData()));
					commdVec.push_back(Command(OPERATOR::DRF));
					assert(!ctool()._context_stk.empty());
					if (ctx.type != Context_Type::ALWAYS)ctool()._context_stk.pop_back();
					// 生成代码
					auto commands = ctx.getCommandSet(ctool()._context_stk, commdVec, word, this);
					for (auto commad : commands) {
						commdVec.push_back(commad);
					}
				}
				else {
					commdVec.push_back(CommandHelper::getPushOpera(word.getData()));	// push 立即数
					assert(!ctool()._context_stk.empty());
					// 如果不是always类型就丢弃这个
					if (ctx.type != Context_Type::ALWAYS) {
						ctool()._context_stk.pop_back();
					}
					// 生成代码
					auto commands = ctx.getCommandSet(ctool()._context_stk, commdVec, word, this);
					for (auto commad : commands) {
						commdVec.push_back(commad);
					}
				}
			}
		}
		else if (type == WordType::IDENTIFIER_SPEC) {
			if (ctx.type != Context_Type::END) {		// 忽略END上下文
#if CHECK_Compiler
				std::cout << "Word = " << word.serialize() << std::endl;
#endif
				assert(!ctool_stk.empty());
				// 直接生成代码
				ctx = helper.get_context(context_name);
				assert(!ctool()._context_stk.empty());
				auto commands = ctx.getCommandSet(ctool()._context_stk, commdVec, word, this);	// 在这里push新的上下文
#if CHECK_Compiler
				show_cstk("Context stk", ctool()._context_stk);
				show_cstk("temp stk", ctool().tempStk);
#endif		
				for (auto commad : commands) {
					commdVec.push_back(commad);
				}

				// 生成前一个上下文的代码
				ctx = ctool()._context_stk.back();
				if (ctx.type != Context_Type::ALWAYS) ctool()._context_stk.pop_back();
				auto commands_last = ctx.getCommandSet(ctool()._context_stk, commdVec, word, this);
				for (auto commad : commands_last) {
					commdVec.push_back(commad);
				}
			}
		}
		else if (type == WordType::OPERATOR_WORD || type == WordType::CONTROLLER) {
#if CHECK_Compiler
			std::cout << "Word = " << word.serialize() << std::endl;
#endif
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
			auto commands = ctx.getCommandSet(ctool()._context_stk, commdVec, word, this);	// 在这里push新的上下文
#if CHECK_Compiler
			std::cout << "Get new Context stack size" << std::endl;
			show_cstk("Context stk", ctool()._context_stk);
			show_cstk("temp stk", ctool().tempStk);
#endif		
			for (auto commad : commands) {
				commdVec.push_back(commad);
			}
		}
	}
END:
	commdVec.push_back(Command(OPERATOR::NOP));
	return;
}