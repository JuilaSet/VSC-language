#pragma once
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <stack>
#include <string>
#include <functional>
#include <initializer_list>

#include "vsEvaluator.h"

#define CHECK_Compiler false
#define CHECK_Compiler_alloc false

// 空上下文
#define EMPTY_CONTEXT \
Context([](ContextStk& cstk, _command_set& _vec, Word& w, auto* compiler) {\
	return _command_set{ };\
})\
//

// 上下文类的类型
enum class Context_Type :int {
	NORMAL,	// 普通类型
	END,	// 结束的上下文
	ALWAYS	// 在遇到上下文结束符前一直存在于栈中的上下文
};

class S_Expr_Compiler;

// 上下文类
class Context {
	friend class S_Expr_Compiler;
	std::function<_command_set(std::vector<Context>&, _command_set&, Word& w, S_Expr_Compiler* p)> _generate;
	std::string name;
	Context_Type type;
public:
	// 空对象
	Context() :
		_generate([](std::vector<Context>&, _command_set&, Word& w, S_Expr_Compiler* p) {return _command_set{}; }),
		type(Context_Type::NORMAL), name("EMPTY_CONTEXT") {};

	Context(
		std::function<_command_set(std::vector<Context>&, _command_set&, Word& w, S_Expr_Compiler* p)> _gen_func, Context_Type _type = Context_Type::NORMAL,
		std::string _name = "Unkown"
	)
		:_generate(_gen_func),
		type(_type),
		name(_name) {}

	inline Context_Type getType() { return type; }
	inline std::string getName() { return name; }
	// 可以操作上下文栈
	_command_set getCommandSet(std::vector<Context>& _context_stk, _command_set& _set, Word& w, S_Expr_Compiler* p) {
		return _generate(_context_stk, _set, w, p);
	}
};
using ContextStk = std::vector<Context>;
using ContextParaOperaFunc = std::function<_command_set(std::vector<Context>&, _command_set&, Word& w, S_Expr_Compiler* p)>;

class Context_error {
public:
	Context_error() = default;
	virtual std::string what() = 0;
};

class Context_found_error : public Context_error {
	friend class Context_helper;
protected:
	std::string error_str;
	Context_found_error(const std::string& str) :error_str(str) {}
public:
	virtual std::string what() {
		return error_str;
	}
};

class Context_helper {
protected:
	// 名称对应的上下文
	std::map<std::string, Context> _context_map;
	std::stack<int> _command_index;
public:
	Context_helper() = default;

	// 存放待确定的指令下标
	void push_command_index(int index) {
#if CHECK_Compiler
		std::cout << std::endl << "push_command_index = " << index << std::endl;
#endif
		_command_index.push(index);
	}

	// 获取待确定的指令下标
	int pop_command_index() {
		int a = _command_index.top();
		_command_index.pop();
#if CHECK_Compiler
		std::cout << std::endl << "pop_command_index = " << a << std::endl;
#endif		
		return a;
	}

	// 注册上下文
	void regist_context(const std::string name, Context gen) {
#if CHECK_Compiler
		std::cerr << "Insert: " << name << std::endl;
#endif
		_context_map.insert(std::make_pair(name, gen));
	}

	// 获取上下文
	Context get_context(const std::string name) throw (Context_error) {
		auto it = _context_map.find(name);
#if CHECK_Compiler
		std::cerr << "try to find: " << name << std::endl;
#endif
		// 如果没有找到对应于name的上下文, 抛出异常
		if (it == _context_map.end()) {
			throw Context_found_error("Context of " + name + " unfound");
		}
#if CHECK_Compiler
		std::cerr << "Success!" << std::endl;
#endif
		return it->second;
	}

	// 建造上下文(从最后一个paras开始)
	Context build_context(ContextParaOperaFunc func, std::initializer_list<Context> ctx_list) {
		std::vector<Context> contexts(ctx_list.begin(), ctx_list.end());
		return Context([=](ContextStk& cstk, _command_set _vec, Word& w,  S_Expr_Compiler* p) {
			_command_set commands = func(cstk, _vec, w, p);
			auto rit = contexts.rbegin();
			auto rend = contexts.rend();
			for (rit; rit != rend; ++rit) {
				cstk.push_back(*rit);
			}
			return commands;
		});
	}
};

#if CHECK_Compiler
void show_cstk(const std::string& tag, std::vector<Context>& stk);
#endif

#if CHECK_Eval_command
void show_comms(const std::vector<Command>& commdVec);
#endif

using word_type_map = std::map<std::string, WordType>;
using local_index_map = std::map<std::string, int>;

// 编译器的工具
class _compiler_tool {
	friend class S_Expr_Compiler;
protected:
	int blk_op_count;				// 用于判断何时在一个block中清除无用数据
	
	std::vector<Context> tempStk;	// 临时栈, tempSTK用于在一个子句结束生成后弹出一个上下文并生成代码

	std::vector<Context> _context_stk;		// 上下文栈

	std::vector<int> paras_count;	// 参数计数栈: 查看操作符参数与上下文结束符是否相符

	int def_stk = 0;				// 判断是否在"定义或赋值语句"的第一个参数中

	std::vector<Word> word_stk;		// 临时存放word

public:

	// 判断是否解引用
	void in_def() {
		def_stk++;
	}
	void out_def() {
		def_stk--;
	}
	bool is_in_def() {
		return def_stk;
	}

	// 参数计数
	void paras_begin() {
		paras_count.push_back(0);
	}
	void paras_end() {
		assert(!paras_count.empty());
		paras_count.pop_back();
	}
	int& paras() {
		return paras_count.back();
	}

	// 翻转代码
	void reserve(std::vector<Command>& commdVec, int size) {
		std::reverse(commdVec.end() - size, commdVec.end());
	}

	// 获取word
	void saveWord(Word w) {
		word_stk.push_back(w);
	}
	Word popWord() {
		Word w = word_stk.back();
		word_stk.pop_back();
		return w;
	}

	void init() {
		_context_stk.clear();
		paras_count.clear();
		def_stk = 0;
		word_stk.clear();
	}
};

// 存放编译结果
class Compile_result {
protected:
	std::map<size_t, vsblock> _sb_map;
	std::vector<Command> _commdVec;
public:
	Compile_result() {}
	~Compile_result() {}

	// 初始化
	void init() {
		_sb_map.clear();
		_commdVec.clear();
	}

	std::vector<Command> getCommandVector() {
		return _commdVec;
	}

	std::vector<Command>& refCommandVector() {
		return _commdVec;
	}

	std::map<size_t, vsblock> getBlockMap() {
		return _sb_map;
	}

	std::map<size_t, vsblock>& refBlockMap() {
		return _sb_map;
	}
};

// 编译器基类
class Basic_Compiler {
public:
	// 生成list_block的代码
	virtual void generate_code(const std::vector<Word>& _word_vector, Compile_result& result, Context_helper& helper)
		throw (Context_error) = 0;

	virtual ~Basic_Compiler() = default;
};

// s-表达式 编译器
class S_Expr_Compiler: public Basic_Compiler
{
public:
	enum { 
		// 分配给标识符的索引最大量
		max_slot_size = 256 
	};
protected:
	std::vector<word_type_map> wt_map_list;		// 存放变量类型
	std::vector<local_index_map> li_map_list;	// 存放标识符到局部变量表的索引映射
	std::vector<_compiler_tool> ctool_stk;		// 编译工具栈， 进入时block入栈, 只对尾部元素操作, 退出block时出栈
	std::vector<vsblock> _vsblock_vec;			// block栈

	// 自动分配变量表的下标(表示符的名称 -> 下标)
	int _auto_alloc_local_index(Word& w) {
		auto& map = li_map_list.back();
		auto serialized_str = w.serialize();

		// 查看是否已经分配过
		auto it = map.find(serialized_str);
		if (it == map.end()) {
			auto index = map.size();
			auto pair = std::make_pair(serialized_str, index); // 从0开始
			map.insert(pair); // 不覆盖原有的map
#if CHECK_Compiler_alloc
			std::cout << "alloc " << serialized_str << " -> " << index << std::endl;
#endif
			assert(max_slot_size >= index);
			return index;
		}
		else {
			// 如果有, 就返回它原来的值
			return it->second;
		}
	}

public:
	// 生成list_block的代码
	virtual void generate_code(const std::vector<Word>& _word_vector, Compile_result& result, Context_helper& helper)
		throw (Context_error);

	inline _compiler_tool& ctool() {
		return ctool_stk.back();
	}
	
	// 判断是否解引用
	void in_def() {
		ctool().in_def();
	}
	void out_def() {
		ctool().out_def();
	}
	bool is_in_def() {
		return ctool().is_in_def();
	}

	// 参数计数
	void paras_begin() {
		ctool().paras_begin();
	}
	void paras_end() {
		ctool().paras_end();
	}
	int& paras() {
		return ctool().paras();
	}

	// 翻转代码
	void reserve(std::vector<Command>& commdVec, int size) {
		ctool().reserve(commdVec, size);
	}

	// 获取word
	void saveWord(Word w) {
		ctool().saveWord(w);
	}
	Word popWord() {
		return ctool().popWord();
	}

	// 变量作用域相关
	void localBegin() {
		wt_map_list.push_back(word_type_map());
		li_map_list.push_back(local_index_map());
	}

	void localEnd() {
		wt_map_list.pop_back();
		li_map_list.pop_back();
	}

	// 插入局部变量记录
	int insert_local(Word& w, WordType type) {
		wt_map_list.back().insert(
			std::make_pair(w.serialize(), type)
		);
		// 分配局部变量表的下标
		return _auto_alloc_local_index(w);
	}

	// 查看当前的IDENTIFIER_ENABLED是否被分配过局部变量下标, 否返回-1, 是返回下标大小
	int get_alloced_index(Word& w) {
		auto rbegin = li_map_list.rbegin();
		auto rend = li_map_list.rend();
		auto serialized_str = w.serialize();
		for (auto it = rbegin;it != rend; ++it) {
			auto fit = it->find(serialized_str);
			if (fit != it->end()) {
				// 存在这个下标
				int index = fit->second;
				return index;
			}
		}
		return -1;
	}

	bool isType(Word& w, WordType type) {
		auto rend = wt_map_list.rend();
		auto rbegin = wt_map_list.rbegin();
		for (auto rit = rbegin; rit != rend; ++rit) {
			auto res = rit->find(w.serialize());
			if (res != rit->end()) {
				if (res->second == type) {
					return true;
				}
				else {
					return false;	// 变量名存放对象的类型不匹配
				}
			}
		}
		return false;	// 没有这个变量名
	}

	void init() {
		ctool().init();
		wt_map_list.clear();
		li_map_list.clear();
	}

	// 
	S_Expr_Compiler();
	virtual ~S_Expr_Compiler();
};