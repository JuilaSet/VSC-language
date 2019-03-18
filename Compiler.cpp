#include "pch.h"
#include "Compiler.h"

Compiler::Compiler()
{
}

Compiler::~Compiler()
{
}

void Compiler::generate_code(const std::vector<Word>& _word_vector, std::vector<Command>& commdVec, _Context_helper& helper) 
	throw (Context_error) {
#if CHECK_Parser
	std::cout << "\n\nGenerate Code Begin:" << std::endl;
#endif
	// 临时栈, tempSTK用于在一个子句结束生成后弹出一个上下文并生成代码
	std::vector<Context> tempStk;
	// 用于判断何时在一个block中清除无用数据的栈
	std::vector<int> blkStk;
	// 根据词_word_vector和上下文栈_context_stk来生成command
	auto begin = _word_vector.begin();
	auto end = _word_vector.end();
	for (auto it = begin; it != end; ++it) {
#if CHECK_Parser
		std::cout << "\n" << std::endl;
#endif		
		Word word = *it;
		// 数字和字符串是基本类型, 直接push进入
		WordType type = word.getType();
		std::string context_name = word.serialize();
		Context ctx;
		if (type == WordType::LOCAL_OPEN) {// blkStk入栈0
			blkStk.push_back(0);
			localBegin();
			// 开启局部变量栈
			commdVec.push_back(Command(OPERATOR::LOCAL_BEGIN));
#if CHECK_Parser
			std::cout << "Word = " << word.serialize() << std::endl;
			show_cstk("Context stk", _context_stk);
			show_cstk("temp stk", tempStk);
#endif		
		}
		else if (type == WordType::LOCAL_CLOSED) {
#if CHECK_Parser
			std::cout << "Word = " << word.serialize() << std::endl;
			show_cstk("Context stk", _context_stk);
			show_cstk("temp stk", tempStk);
#endif		// blkStk出栈
			blkStk.pop_back();
			// 关闭局部变量栈
			commdVec.push_back(Command(OPERATOR::LOCAL_END));

			// 当做上下文结束符";"处理		
#if CHECK_Parser
			std::cout << "CONTEXT_CLOSED" << std::endl;
#endif	
			// pop到END处为止, 忽略之后所有的参数
			while (ctx.type != Context_Type::END) {
				// 如果为空就结束
				if (!_context_stk.empty()) {
					ctx = _context_stk.back();
					_context_stk.pop_back();
#if CHECK_Parser
					std::cout << "Context stack POP = " << ctx.getName() << std::endl;
					show_cstk("Context stk", _context_stk);
					show_cstk("temp stk", tempStk);
#endif
				}
				else {
					// 如果为空, 结束
					goto END;
				}
			}

			// 生成代码(这里的上下文一定是END类型)
			auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);
			for (auto commad : commands) {
				commdVec.push_back(commad);
			}

			// 如果之前保存了外层上下文, 就生成外部的COMMAND
			if (!tempStk.empty()) {
				ctx = tempStk.back();
				tempStk.pop_back();
#if CHECK_Parser
				std::cout << "TEMP stack POP = " << ctx.getName() << std::endl;
				show_cstk("Context stk", _context_stk);
				show_cstk("temp stk", tempStk);
#endif
				// 生成代码
				auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);
				for (auto commad : commands) {
					commdVec.push_back(commad);
				}
			}

			// 判断是否生成SHRINK指令
			assert(!blkStk.empty());
			if ((blkStk.back() -= 1) == 0) {
				commdVec.push_back(Command(OPERATOR::SHRINK));
			}
			localEnd();
		}
		else if (type == WordType::STRING_OPEN || type == WordType::STRING_CLOSED) {
#if CHECK_Parser
			std::cout << "Word = " << word.serialize() << std::endl;
			show_cstk("Context stk", _context_stk);
			show_cstk("temp stk", tempStk);
#endif
			// 什么都不做
		}
		else if (type == WordType::CONTEXT_CLOSED) {
			// 遇到上下文结束符";"
#if CHECK_Parser
			std::cout << "Word = " << word.serialize() << std::endl;
#endif	
			// pop到END处为止, 忽略之后所有的参数
			while (ctx.type != Context_Type::END) {
				// pop
				assert(!_context_stk.empty());
				ctx = _context_stk.back();
				_context_stk.pop_back();
#if CHECK_Parser
				std::cout << "Context stack POP = " << ctx.getName() << std::endl;
				show_cstk("Context stk", _context_stk);
				show_cstk("temp stk", tempStk);
#endif
			}

			// 生成代码
			auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);
			for (auto commad : commands) {
				commdVec.push_back(commad);
			}

			// 如果之前保存了外部op, 就生成外部op的COMMAND
			if (!tempStk.empty()) {
				ctx = tempStk.back();
				tempStk.pop_back();
#if CHECK_Parser
				std::cerr << "TEMP STK POP = " << ctx.getName() << std::endl;
				show_cstk("Context stk", _context_stk);
				show_cstk("temp stk", tempStk);
#endif		
				// 生成代码
				auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);
				for (auto commad : commands) {
					commdVec.push_back(commad);
				}
			}

			// 判断是否生成SHRINK指令
			assert(!blkStk.empty());
			if ((blkStk.back() -= 1) == 0) {
				commdVec.push_back(Command(OPERATOR::SHRINK));
			}
		}
		else if (type == WordType::NUMBER || type == WordType::STRING || type == WordType::IDENTIFIER_ENABLED) {	// 新增, 对基本类型的操作
#if CHECK_Parser
			std::cout << "Word = " << word.serialize() << std::endl;
#endif		
			ctx = _context_stk.back();
			if (ctx.type != Context_Type::END) {		// 忽略END上下文
			// pop
#if CHECK_Parser
				std::cout << "Context stack POP = " << ctx.getName() << std::endl;
				std::cout << "COMMAND: PUSH " << context_name << std::endl;
				show_cstk("Context stk", _context_stk);
				show_cstk("temp stk", tempStk);
#endif
				if (type == WordType::IDENTIFIER_ENABLED && !is_in_def()) {
					// 如果不在def或assign语句的第一个参数内, 生成解引用代码(即使是事先没有定义)
					// && isType(word, WordType::IDENTIFIER)
					ctx = _context_stk.back();
#if CHECK_Parser
					std::cout << "如果已经定义了, 并且不在def语句的第一个参数内, 生成解引用代码" << std::endl;
					std::cout << "Context stack POP " << ctx.getName() << std::endl;
					show_cstk("Context stk", _context_stk);
					show_cstk("temp stk", tempStk);
#endif
					// pop
					// 生成解引用代码
					commdVec.push_back(CommandHelper::getPushOpera(word.getData()));
					commdVec.push_back(Command(OPERATOR::DRF));
					assert(!_context_stk.empty());
					if (ctx.type != Context_Type::ALWAYS)_context_stk.pop_back();
					// 生成代码
					auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);
					for (auto commad : commands) {
						commdVec.push_back(commad);
					}
				}
				else {
					commdVec.push_back(CommandHelper::getPushOpera(word.getData()));	// push 立即数
					assert(!_context_stk.empty());
					// 如果不是always类型就丢弃这个
					if (ctx.type != Context_Type::ALWAYS) {
						_context_stk.pop_back();
					}
					// 生成代码
					auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);
					for (auto commad : commands) {
						commdVec.push_back(commad);
					}
				}
			}
		}
		else if (type == WordType::IDENTIFIER_SPEC) {
			if (ctx.type != Context_Type::END) {		// 忽略END上下文
#if CHECK_Parser
				std::cout << "Word = " << word.serialize() << std::endl;
#endif
				// 直接生成代码
				ctx = helper.get_context(context_name);
				assert(!_context_stk.empty());
				auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);	// 在这里push新的上下文
#if CHECK_Parser
				show_cstk("Context stk", _context_stk);
				show_cstk("temp stk", tempStk);
#endif		
				for (auto commad : commands) {
					commdVec.push_back(commad);
				}

				// 生成前一个上下文的代码
				ctx = _context_stk.back();
				if (ctx.type != Context_Type::ALWAYS) _context_stk.pop_back();
				auto commands_last = ctx.getCommandSet(_context_stk, commdVec, word, this);
				for (auto commad : commands_last) {
					commdVec.push_back(commad);
				}
			}
		}
		else if (type == WordType::OPERATOR_WORD || type == WordType::CONTROLLER) {
#if CHECK_Parser
			std::cout << "Word = " << word.serialize() << std::endl;
#endif
			// pop to tempstk
			if (!_context_stk.empty()) {	// void -> atomic
				ctx = _context_stk.back();
				if (ctx.type != Context_Type::END) {		// 忽略END类型
					tempStk.push_back(ctx);
					if (ctx.type != Context_Type::ALWAYS) _context_stk.pop_back();
#if CHECK_Parser
					std::cout << "Context = " << ctx.getName() << " stack POP into Temp" << std::endl;
					show_cstk("Context stk", _context_stk);
					show_cstk("temp stk", tempStk);
#endif
				}
			}

			// 操作符栈顶+1
			assert(!blkStk.empty());
			blkStk.back() += 1;

			// 生成代码
			ctx = helper.get_context(context_name);
			auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);	// 在这里push新的上下文
#if CHECK_Parser
			std::cout << "Get new Context stack size" << std::endl;
			show_cstk("Context stk", _context_stk);
			show_cstk("temp stk", tempStk);
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