#include "pch.h"
#include "Parser.h"

#if	CHECK_Parser
void _BNF_Node::display() {

	std::cout << "(" << _name << (static_cast<bool>(_isTerminal) ? "" : " g: " + _g_name) << ", nexts: ";
	for (auto next : _nexts) {
		std::cout << next << ", ";
	}
	std::cout << ")";
}
#endif

#if	CHECK_Parser
void BNFGraphic::display() {
	std::cout << "[G: " << _name << std::endl;
	for (int i = 0, size = _nodes.size(); i < size; ++i) {
		std::cout << "node_" << i << ": ";
		_nodes[i].display();
		std::cout << std::endl;
	}
	std::cout << "]" << std::endl;
}
#endif

#define GHEAD "_head"
#define GEND "_end"

Graphic_builder* Graphic_builder::gotoNode(const std::string nodeName) {
	_BNF_Node node(IsTerminal::False, _bnf_graphic._name + "_" + nodeName);
	auto index = _nodes_index_map.find(node);
	assert(index != _nodes_index_map.end());
	_cur_node = index->second;
	return this;
}

Graphic_builder* Graphic_builder::rule() {
	_BNF_Node node_head(IsTerminal::True, _bnf_graphic._name + GHEAD, [](Word w, std::string& err, int lex_point) {return false; });
	_bnf_graphic._nodes.push_back(node_head);	// 首结点0
	_nodes_index_map.insert(std::make_pair(node_head, 0));

	_BNF_Node node_end(IsTerminal::True, _bnf_graphic._name + GEND, [](Word w, std::string& err, int lex_point) {return false; });
	_bnf_graphic._nodes.push_back(node_end);	// 尾结点1
	_nodes_index_map.insert(std::make_pair(node_end, 1));
	_cur_node = 0;
	return this;
}

Graphic_builder* Graphic_builder::terminal(std::string nodeName, judge_func func) {
	int cur_index;
	_BNF_Node node(IsTerminal::True, _bnf_graphic._name + "_" + nodeName, func);
	auto it = _nodes_index_map.find(node);
	if (it == _nodes_index_map.end()) {
		_bnf_graphic._nodes.push_back(node);
		cur_index = _bnf_graphic._nodes.size() - 1;
		_bnf_graphic._nodes[_cur_node]._nexts.push_back(cur_index);
		_nodes_index_map.insert(std::make_pair(node, cur_index));
		_cur_node = cur_index;
	}
	else {	// 已经存在这个结点
		cur_index = it->second;
		_bnf_graphic._nodes[_cur_node]._nexts.push_back(cur_index);
	}
	_cur_node = cur_index;
	return this;
}

Graphic_builder* Graphic_builder::nonterminal(std::string nodeName, std::string graphicName) {
	int cur_index;
	_BNF_Node node(IsTerminal::False, _bnf_graphic._name + "_" + nodeName);
	node._g_name = graphicName;
	auto it = _nodes_index_map.find(node);
	if (it == _nodes_index_map.end()) {
		_bnf_graphic._nodes.push_back(node);
		cur_index = _bnf_graphic._nodes.size() - 1;
		_bnf_graphic._nodes[_cur_node]._nexts.push_back(cur_index);
		_nodes_index_map.insert(std::make_pair(node, cur_index));
		_cur_node = cur_index;
	}
	else {	// 已经存在这个结点
		cur_index = it->second;
		_bnf_graphic._nodes[_cur_node]._nexts.push_back(cur_index);
	}
	_cur_node = cur_index;
	return this;
}

Graphic_builder* Graphic_builder::gotoHead() {
	_cur_node = 0;
	return this;
}

Graphic_builder* Graphic_builder::end() {
	_bnf_graphic._nodes[_cur_node]._nexts.push_back(1);
	_cur_node = 1;
	return this;
}

// parser
Parser::Parser(Lexer* lex):_lexer(lex) {

}

Parser::~Parser() {

}

void Parser::addBNFRuleGraphic(BNFGraphic g) {
	this->_graphic_map.emplace(make_pair(g.getId(), g));
}

// 分析图是否匹配
int Parser::_judge_g(Position pos, std::string& errors) {
	std::string failed_info;
	OperatorParas_vec opera_paras;	// operator 以及相应参数

	Position _cur_position = pos;
	_cur_position._node_index = 0;
	int offset_start = pos._lexer_point;
	while (1) {
#if CHECK_Parser
		std::cout << "\n\n" << __LINE__  << "\t"  << "In BNFGraphic: [" << _cur_position._graphic_str << "]" << std::endl;
#endif
		// 找到当前的图
		assert(_cur_position._graphic_str != "");
		assert(_graphic_map.find(_cur_position._graphic_str) != _graphic_map.end());
		BNFGraphic g = _graphic_map.find(_cur_position._graphic_str)->second;
		// 从lexer中取出word
		Word word = _lexer->getWord(_cur_position._lexer_point);
#if CHECK_Parser
		std::cout << __LINE__  << "\t"  << "Get Word from lexer: " << word.serialize() << std::endl;
#endif
		// 找到符合word的结点
		bool found = false;
		bool end = false;
		
		assert(g._nodes.size() > _cur_position._node_index);
		_BNF_Node node = g.getNode(_cur_position._node_index);
		int _offset = 0;
		for (int i : node._nexts) {
			_BNF_Node child = g.getNode(i);
#if CHECK_Parser
			std::cout << __LINE__  << "\t"  << "Get Node" << std::endl;
			child.display();
			std::cout << std::endl << "Judging: " << std::endl;
#endif
			if (i == 1) {
#if CHECK_Parser
				std::cout << __LINE__  << "\t"  << "Had Path to end" << std::endl;
#endif
				end = true;
			} else { // 如果不是结束
				if (child.isTerminal()) {	// 终结符
#if CHECK_Parser
					std::cout << __LINE__  << "\t"  << "Terminal node:" << std::endl;
#endif
					if (child.judge(word, errors, _cur_position._lexer_point)) {
#if CHECK_Parser
						std::cout << __LINE__  << "\t"  << "Terminal node judge Success!" << std::endl;
#endif
						found = true;
						_offset = 1;
						_cur_position._node_index = i;
						break;
					}
				}
				else {
#if CHECK_Parser
					std::cout << __LINE__  << "\t"  << "NON - Terminal node: jump to" << std::endl;
#endif
					// 非终结符 跳转图
					assert(child.getGName() != "");
					if (_offset = _judge_g(Position(0, child.getGName(), _cur_position._lexer_point), errors)) {
#if CHECK_Parser
						std::cout << __LINE__  << "\t"  << "NON - Terminal node Judge Success!" << std::endl;
#endif
						// 判断成功
						found = true;
						_cur_position._node_index = i;
						break;
					}
				}
			}
		}
		// 如果没有符合的非结束结点
		if (!found) {
#if CHECK_Parser
			std::cout << __LINE__  << "\t"  << "Found failed!" << std::endl;
#endif
			if (end) goto Success;	// 如果可到达结束结点,成功
			else goto Failed;		// 没有可到达结束位置的路, 失败
		}
		else {
			// 如果符合条件的结点是EOS, 那么成功
			if (word.getType() == WordType::EOS) {
				goto Success;
			}
			else {
#if CHECK_Parser
				std::cout << __LINE__  << "\t"  << "Step to next word: Step = " << _offset << std::endl;
#endif
				// 否则继续读入word
				_cur_position._lexer_point += _offset;
			}
		}
	}
Failed:
#if CHECK_Parser
	std::cout << __LINE__  << "\t"  << "Failed and return!" << std::endl;
#endif
	return 0;
Success:
	errors = "";// 正确时忽略之前的错误
	int offset_ret = _cur_position._lexer_point - offset_start;
#if CHECK_Parser
	std::cout << __LINE__  << "\t"  << "Success and return! offset = " << offset_ret << std::endl;
#endif
	// 语法成功
	return offset_ret; // 返回偏移量
}

// 语法分析
bool Parser::parse(std::string graphicId) {
	std::string err = "";
	// 递归下降判断
	int offset = _judge_g(Position(0, graphicId, 0), err);

	for (int i = 0; i < offset; ++i) {
		// 放入修改后的code
		_word_vector.push_back(_lexer->getWord(i));
	}
#if CHECK_Parser
	std::cerr << (offset > 0 ? "Parse Sucess!" : "Parse Failed!") << std::endl;
#endif
	this->errors = err;
	return offset > 0;
}

// 生成list_block的代码
void Parser::generate_code(std::vector<Command>& commdVec, _Context_helper& helper) {
#if CHECK_Parser
	std::cout << "\n\nGenerate Code Begin:" << std::endl;
#endif
	// 临时栈, tempSTK用于在一个子句结束生成后弹出一个上下文并生成代码
	std::stack<Context> tempStk;
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
		if (type == WordType::LOCAL_OPEN) {
#if CHECK_Parser
			std::cout << "WordType:LOCAL_OPEN: " << std::endl;
			std::cout << "push++++<Temp Stk size> = " << tempStk.size() << std::endl;
#endif		// blkStk入栈0
			blkStk.push_back(0);
			localBegin();
			// 开启局部变量栈
			commdVec.push_back(Command(OPERATOR::LOCAL_BEGIN));
		}
		else if (type == WordType::LOCAL_CLOSED) {
#if CHECK_Parser
			std::cout << "WordType:LOCAL_CLOSED: " << std::endl;
			std::cout << "pop++++<Temp Stk size> = " << tempStk.size() << std::endl;
#endif		// blkStk出栈
			blkStk.pop_back();
			localEnd();
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
					ctx = _context_stk.top();
#if CHECK_Parser
					std::cout << "Context stack size = " << _context_stk.size() << std::endl;
					std::cout << "Context stack POP = " << ctx.getName() << std::endl;
#endif
					_context_stk.pop();
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

			// 如果之前保存了外部op, 就生成外部op的COMMAND
			if (!tempStk.empty()) {
				ctx = tempStk.top();
#if CHECK_Parser
				std::cout << "Temp stack Size = " << tempStk.size() << std::endl;
				std::cout << "Temp stack POP = " << ctx.getName() << std::endl;
#endif
				tempStk.pop();
				// 生成代码
				auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);
				for (auto commad : commands) {
					commdVec.push_back(commad);
				}
			}
		}
		if (type == WordType::STRING_OPEN || type == WordType::STRING_CLOSED) {
#if CHECK_Parser
			std::cout << "WordType:LOCAL_OPEN: " << std::endl;
#endif
			// 什么都不做
		}
		else if (type == WordType::CONTEXT_CLOSED) {
			// 遇到上下文结束符";"		
#if CHECK_Parser
			std::cout << "CONTEXT_CLOSED" << std::endl;
			std::cout << "WordType:CONTEXT_CLOSED: " << std::endl;
			std::cout << "++++Temp Stk size = " << tempStk.size() << std::endl;
#endif	
			// pop到END处为止, 忽略之后所有的参数
			while (ctx.type != Context_Type::END) {
				// pop
				assert(!_context_stk.empty());
				ctx = _context_stk.top();
#if CHECK_Parser
				std::cout << "Context stack size = " << _context_stk.size() << std::endl;
				std::cout << "Context stack POP = " << ctx.getName() << std::endl;
#endif
				_context_stk.pop();
			}

			// 生成代码
			auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);
			for (auto commad : commands) {
				commdVec.push_back(commad);
			}

			// 如果之前保存了外部op, 就生成外部op的COMMAND
			if (!tempStk.empty()) {
				ctx = tempStk.top();
#if CHECK_Parser
				std::cout << "Temp stack Size = " << tempStk.size() << std::endl;
				std::cout << "Temp stack POP = " << ctx.getName() << std::endl;
#endif		
				tempStk.pop();
				// 生成代码
				auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);
				for (auto commad : commands) {
					commdVec.push_back(commad);
				}
			}

			// 判断是否生成POP指令
			if ((blkStk.back() -= 1) == 0) {
				commdVec.push_back(Command(OPERATOR::SHRINK));
			}
		}
		else if (type == WordType::NUMBER || type == WordType::STRING || type == WordType::IDENTIFIER_ENABLED) {	// 新增, 对基本类型的操作
#if CHECK_Parser
			std::cout << "WordType:: NUMBER : STRING : IDENTIFIER_ENABLED: " << std::endl;
#endif		
			ctx = _context_stk.top();
			if (ctx.type != Context_Type::END) {		// 忽略END上下文
			// pop
#if CHECK_Parser
				std::cout << "Context stack size = " << _context_stk.size() << std::endl;
				std::cout << "Context stack POP = " << ctx.getName() << std::endl;
				std::cout << "COMMAND: PUSH " << context_name << std::endl;
#endif
				if (type == WordType::IDENTIFIER_ENABLED && isType(word, WordType::IDENTIFIER) && !is_in_def()) {
					// 如果已经定义了, 并且不在def语句的第一个参数内, 生成解引用代码
					ctx = _context_stk.top();
#if CHECK_Parser
					std::cout << "wordType::IDENTIFIER: " << std::endl;
					std::cout << "Context stack size = " << _context_stk.size() << std::endl;
					std::cout << "Context stack POP " << ctx.getName() << std::endl;
#endif
					// pop
					// 生成解引用代码
					commdVec.push_back(CommandHelper::getPushOpera(word.getData()));
					commdVec.push_back(Command(OPERATOR::DRF));
					assert(!_context_stk.empty());
					if (ctx.type != Context_Type::ALWAYS)_context_stk.pop();
					// 生成代码
					auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);
					for (auto commad : commands) {
						commdVec.push_back(commad);
					}
				}
				else {
					commdVec.push_back(CommandHelper::getPushOpera(word.getData()));	// push 立即数
					assert(!_context_stk.empty());
					if(ctx.type != Context_Type::ALWAYS) _context_stk.pop();
					// 生成代码
					auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);
					for (auto commad : commands) {
						commdVec.push_back(commad);
					}
				}
			}
		}
		else if (type == WordType::OPERATOR_WORD || type == WordType::CONTROLLER) {
#if CHECK_Parser
			std::cout << "wordType::KEY_WORD: " << std::endl;
			std::cout << "Context stack size = " << _context_stk.size() << std::endl;
#endif
			// pop to tempstk
			if (!_context_stk.empty()) {	// void -> atomic
				ctx = _context_stk.top();
				if (ctx.type != Context_Type::END) {		// 忽略END类型
					tempStk.push(ctx);
					if (ctx.type != Context_Type::ALWAYS) _context_stk.pop();
#if CHECK_Parser
					std::cout << "Context = " << ctx.getName() << " stack POP into Temp" << std::endl;
					std::cout << "Temp stack Size = " << tempStk.size() << std::endl;
#endif
				}
			}
			
			// 操作符+1
			blkStk.back() += 1;
			
			// 生成代码
			ctx = helper.get_context(context_name);
			auto commands = ctx.getCommandSet(_context_stk, commdVec, word, this);	// 在这里push新的上下文
#if CHECK_Parser
			std::cout << "Get new Context stack size = " << _context_stk.size() << std::endl;
#endif		
			for (auto commad : commands) {
				commdVec.push_back(commad);
			}
		}
	}
END:
	return;
}
