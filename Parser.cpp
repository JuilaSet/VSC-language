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
int Parser::_judge_g(Position pos, std::string& errors, std::vector<Word>& _judge_vector) {
	std::string failed_info;
	OperatorParas_vec opera_paras;	// operator 以及相应参数
	// std::vector<Word> _temp_judge_vector; // 当前图的vector, 如果失败就丢弃, 成功就加入_judge_vector

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
						// 正确的时候加入搜索路径
						_judge_vector.push_back(word);
						break;
					}
				}
				else {
#if CHECK_Parser
					std::cout << __LINE__  << "\t"  << "NON - Terminal node: jump to" << std::endl;
#endif
					// 非终结符 跳转图进行判断
					assert(child.getGName() != "");
					if (_offset = _judge_g(Position(0, child.getGName(), _cur_position._lexer_point), errors, _judge_vector)) {
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
	// 擦除错误路径
	_judge_vector.erase(_judge_vector.begin() + offset_start, _judge_vector.end());
	return 0;

Success:
	// 正确时忽略之前的错误, 并将搜索到的word放入表中
	errors = "";
	int offset_ret_SUCESS = _cur_position._lexer_point - offset_start;
#if CHECK_Parser
	std::cout << __LINE__  << "\t"  << "Success and return! offset = " << offset_ret << std::endl;
#endif
	// 语法成功
	return offset_ret_SUCESS; // 返回偏移量
}

// 语法分析(true or false)
bool Parser::parse(std::string graphicId) {
	std::string err = "";
	std::vector<Word> _judge_vector;

	// 递归下降判断
	int offset = _judge_g(Position(0, graphicId, 0), err, _judge_vector);

#if CHECK_Parser_g
	// 显示结果
	std::cout << "parser的结果: " << std::endl;
	std::for_each(_judge_vector.begin(), _judge_vector.end(), [](auto& ele) {
		std::cout << ele.getString() << " ";
	});
	std::cout << std::endl;
#endif

	// 调整修改后的code加入 _word_vector
	for (int i = 0; i < offset; ++i) {
		_word_vector.push_back(_judge_vector[i]);
	}

	// 清空分析结果
	_judge_vector.clear();

#if CHECK_Parser
	std::cerr << (offset > 0 ? "Parse Sucess!" : "Parse Failed!") << std::endl;
#endif
	this->errors = err;
	return offset > 0;
}
