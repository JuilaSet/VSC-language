#pragma once
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <stack>
#include <string>
#include <functional>
#include <initializer_list>
#include "Compiler.h"

#define CHECK_Parser false
#define CHECK_Parser_g true

enum class IsTerminal : bool {
	False = false, True
};

using OperatorParas_vec = std::vector<int>;
using judge_func = std::function<bool(Word, std::string& err, int lex_point)>;
class _BNF_Node {
	friend class Parser;
	friend class Graphic_builder;
	friend class BNFGraphic;
	friend struct _BNF_Node_Hash;
protected:
	IsTerminal _isTerminal;		// 是否是终结的
	std::string _name;			// 结点名称
	std::vector<int> _nexts;	// 后继下标
	judge_func _judge;			// 判断函数
	std::string _g_name;		// _isTerminal为假时有效, 非终结结点代表的图的名称
public:
	_BNF_Node(IsTerminal isTerminal, std::string id, judge_func lambda_judge = judge_func(), std::vector<int> nexts = {}) :
		_isTerminal(isTerminal),
		_name(id),
		_judge(lambda_judge),
		_nexts(nexts) {}
	bool isTerminal(){
		return static_cast<bool>(_isTerminal);
	}
	bool judge(Word w, std::string& err, int lex_point) const {
		return _judge(w, err, lex_point);
	}
	std::string getGName() {
		assert(!(bool)_isTerminal);
		return _g_name;
	}
	
	// 重载==运算符
	bool operator==(const _BNF_Node& n) const {
		return _name == n._name;
	}
#if	CHECK_Parser
	void display();
#endif
};

// hash函数对象
struct _BNF_Node_Hash
{
	size_t operator()(const _BNF_Node &n) const
	{
		return n._name.size();	//按照名字的长度当做hash值
	}
};

class Graphic_builder;
// BNFGraphic
class BNFGraphic {
	friend class Graphic_builder;
	friend class Parser;
protected:
	std::string _name;	// 名称, 唯一
	std::vector<_BNF_Node> _nodes;
public:
	BNFGraphic(std::string id) :_name(id) { }
	std::string getId() { return _name; }
	_BNF_Node getNode(int index) { return _nodes[index]; }
	_BNF_Node getHead() { return _nodes[0]; }	// 首结点
	_BNF_Node getEnd() { return _nodes[1]; }	// 尾结点
	void reset(std::string GraphicName) {
		_name = GraphicName;
		_nodes.clear();
	}
#if	CHECK_Parser
	void display();
#endif
};

class Graphic_builder {
protected:
	BNFGraphic _bnf_graphic;
	std::unordered_map<_BNF_Node, int, _BNF_Node_Hash> _nodes_index_map;	// 结点所处下标
	int _cur_node;		// 当前操作结点
public:
	Graphic_builder(std::string GraphicName) :_bnf_graphic(GraphicName) { }
	void reset(std::string GraphicName) {
		_bnf_graphic.reset(GraphicName);
		_nodes_index_map.clear();
	}
	BNFGraphic getGraphic() { return (_bnf_graphic); }
	Graphic_builder* gotoNode(const std::string nodeName);
	Graphic_builder* rule();	// 新建头结点
	Graphic_builder* terminal(const std::string nodeName, judge_func func = [](Word, std::string& err, int lex_point) {return false; });
	Graphic_builder* nonterminal(const std::string nodeName, const std::string graphicName);
	Graphic_builder* gotoHead();
	Graphic_builder* end();
};

// Position
struct Position {
	unsigned int _node_index;	// 所在结点下标
	std::string _graphic_str;	// 所在图字符串
	unsigned int _lexer_point;
	
	Position() = default;

	Position(unsigned int index, std::string gid, unsigned int _lex_point):
		_node_index(index), _graphic_str(gid), _lexer_point(_lex_point){}

	Position(const Position& pos) {
		_node_index = pos._node_index;
		_graphic_str = pos._graphic_str;
		_lexer_point = pos._lexer_point;
	}
};

class TNode {
	Position _position;
	int _depth;
	int _father;
};

using word_type_map = std::map<std::string, WordType>;

// 语法分析器
class Parser
{
protected:
	std::vector<Word> _word_vector;	// 保存解析完成的代码
	std::vector<Word> _judge_vector;	// 保存解析完成的代码

	std::string errors;				// 存放分析错误
	Lexer* _lexer;					// 外部对象, 不需要删除析构
	std::map<std::string, BNFGraphic> _graphic_map;		// 名称到图的map
	int _judge_g(Position pos, std::string& errors, std::vector<Word>& _judge_vector);	// 根据图判断语法正确性

public:
	Parser(Lexer* lex);
	std::string getErrors() { return errors; }
	void init() {
		_word_vector.clear();
	}
	void addBNFRuleGraphic(BNFGraphic g);
	bool parse(std::string firstGraphicId);
	
	// 返回word_vector的引用
	std::vector<Word>& get_word_vector_ref() {
		return _word_vector;
	}
	virtual ~Parser();
};
