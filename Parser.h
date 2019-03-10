#pragma once
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <stack>
#include <string>
#include <functional>
#include <initializer_list>
#include "Evaluator.h"

#define CHECK_Parser false

#define EMPTY_CONTEXT \
Context([](ContextStk& cstk, _command_set& _vec, Word& w) {\
	return _command_set{ };\
})\
//

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

struct Position {
	unsigned int _node_index;					// 所在结点下标
	std::string _graphic_str;				// 所在图字符串
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

// 上下文类的类型
enum class Context_Type :int {
	NORMAL,	// 普通类型
	END,	// 结束的上下文
};

// 上下文类
using _command_set = std::vector<Command>;
class Context {
	friend class Parser;
	std::function<_command_set(std::stack<Context>&, _command_set&, Word& w)> _generate;
	std::string name;
	Context_Type type;
public:
	// 空对象
	Context() :
		_generate([](std::stack<Context>&, _command_set&, Word& w) {return _command_set{}; }),
		type(Context_Type::NORMAL), name("EMPTY_CONTEXT"){};

	Context(
		std::function<_command_set(std::stack<Context>&, _command_set&, Word& w)> _gen_func, Context_Type _type = Context_Type::NORMAL,
		std::string _name = "Unkown"
	)
		:_generate(_gen_func),
		type(_type),
		name(_name){}

	inline Context_Type getType() { return type; }
	inline std::string getName() { return name; }
	// 可以操作上下文栈
	_command_set getCommandSet(std::stack<Context>& _context_stk, _command_set& _set, Word& w) {
		return _generate(_context_stk, _set, w);
	}
};
using ContextStk = std::stack<Context>;
using ContextParaOperaFunc = std::function<_command_set(std::stack<Context>&, _command_set&, Word& w)>;

class _Context_helper {
protected:
	// 名称对应的上下文
	std::map<std::string, Context> _context_map;
	std::stack<int> _command_index;
public:
	_Context_helper() = default;

	// 存放待确定的指令下标
	void push_command_index(int index) {
		_command_index.push(index);
	}

	// 获取待确定的指令下标
	int pop_command_index() {
		int a = _command_index.top();
		_command_index.pop();
		return a;
	}

	// 注册上下文
	void regist_context(const std::string name, Context gen) {
#if CHECK_Parser
		std::cerr << "Insert: " << name << std::endl;
#endif
		_context_map.insert(std::make_pair(name, gen));
	}

	// 获取上下文
	Context get_context(const std::string name) {
		auto it = _context_map.find(name);
#if CHECK_Parser
		std::cerr << "try to find: " << name << std::endl;
#endif
		assert(it != _context_map.end());
#if CHECK_Parser
		std::cerr << "Success!" << std::endl;
#endif
		return it->second;
	}

	// 建造上下文(从最后一个paras开始)
	Context build_context(ContextParaOperaFunc func, std::initializer_list<Context> ctx_list) {
		std::vector<Context> contexts(ctx_list.begin(), ctx_list.end());
		return Context([=](ContextStk& cstk, _command_set _vec, Word& w) {
			_command_set commands = func(cstk, _vec, w);
			auto rit = contexts.rbegin();
			auto rend = contexts.rend();
			for (rit; rit != rend; ++rit) {
				cstk.push(*rit);
			}
			return commands;
		});
	}
};

//
class Parser
{
public: // [protected]][
	std::vector<Word> _word_vector;	// 保存解析完成的中间代码
protected:
	std::string errors;				// 存放分析错误
	Lexer* _lexer;					// 外部对象, 不需要删除析构
	std::map<std::string, BNFGraphic> _graphic_map;	// 名称到图的map
	int _judge_g(Position pos, std::string& errors);				// 根据图判断语法正确性

	std::stack<Context> _context_stk;		// 上下文栈
public:
	Parser(Lexer* lex);
	std::string getErrors() { return errors; }
	void init() {
		_word_vector.clear();
		while(!_context_stk.empty())_context_stk.pop();
	}
	void addBNFRuleGraphic(BNFGraphic g);
	bool parse(std::string firstGraphicId);
	void generate_code(std::vector<Command>& commdVec, _Context_helper& helper);
	virtual ~Parser();
};

namespace Context_Helper {
	// 全局变量
	static _Context_helper helper;

}