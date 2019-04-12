#pragma once

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
using _command_set = std::vector<Command>;

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
using index_t = long int;
using word_type_map = std::map<std::string, WordType>;
using local_index_map = std::map<std::string, int>;
using form_paras_map = std::map<std::string, int>;	// 存放标识符到形参表的索引映射

// 临时状态信息
class _temp_compiler_tool {
	bool _sub_strong_hold = false;	// 下一个作用域是否是强作用域
	form_paras_map _next_form_map;	// 当前作用域的形式参数映射(参数名 -> 参数地址)
public:
	// 设置作用域
	void setSubFieldStrongHold(bool b) { _sub_strong_hold = b; }
	bool subFieldStrongHold() const { return _sub_strong_hold; }

	// 返回map引用
	form_paras_map& next_form_paras_map() {
		return _next_form_map;
	}

	// 添加形参
	int add_para(Word& w) {
		int index = _next_form_map.size();
		auto pair = std::make_pair(w.serialize(), index + 1); // 从1开始，0保留给函数自己用
		_next_form_map.insert(pair);
		return index;
	}

	// 查询形参地址
	int indexof(Word& w) {
		// 查询
		auto it = _next_form_map.find(w.serialize());
		if (it != _next_form_map.end()) {
			return it->second;
		}
		// 失败返回-1
		return -1;
	}

	// 初始化
	void init() {
#if CHECK_Compiler_Field
		std::cout << __LINE__ << " 初始化临时作用域为false" << std::endl;
#endif
		this->_sub_strong_hold = false;
		_next_form_map.clear();
	}
};

// 编译器的工具
class _compiler_tool {
	friend class S_Expr_Compiler;
protected:
	int blk_op_count;				// 用于判断何时在一个block中清除无用数据

	std::vector<Context> tempStk;	// 临时栈, tempSTK用于在一个子句结束生成后弹出一个上下文并生成代码

	std::vector<Context> _context_stk;	// 上下文栈

	std::vector<int> paras_count;	// 指令参数计数栈: 查看操作符参数与上下文结束符是否相符

	form_paras_map _form_map;		// 当前作用域的形式参数映射(参数名 -> 参数地址)

	int def_stk = 0;				// 判断是否在"定义或赋值语句"的第一个参数中

	int _def_para = 0;				// 判断是否是定义形参

	bool _is_def_blk = false;		// 判断是否是定义过程， 而不是执行过程

	bool _strong_hold = false;		// 当前作用域是否是强作用域

	std::vector<Word> word_stk;		// 临时存放word

	_temp_compiler_tool _temp_tool; // 临时状态信息
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

	// 判断是否生成call_blk语句
	void in_def_blk() { _is_def_blk = true; }
	void out_def_blk() { _is_def_blk = false; }
	bool is_def_blk() const { return _is_def_blk; }

	// 判断IDENTIFIER_ENABLED word是否是形参声明
	void def_paras_begin() {
		_def_para++;
	}
	void def_paras_end() {
		_def_para--;
	}
	bool is_def_paras() {
		return _def_para;
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

	// 当前是否是强作用域
	bool strong_hold() { return _strong_hold; }

	// 返回临时信息
	inline _temp_compiler_tool& get_temp_tool_ref() {
		return _temp_tool;
	}

	// 将临时信息传递给自己, 应该在每一次新建ctool的时候传递
	void pass_temp_message(_temp_compiler_tool& temp_tool) {
		_strong_hold = temp_tool.subFieldStrongHold();
		// 复制形参下标
		auto& map = temp_tool.next_form_paras_map();
		_form_map.insert(map.begin(), map.end());
		// ...

		// 传递完后初始化
		temp_tool.init();
	}

	// 查询形参地址
	int indexof_form_para(Word& w) {
		// 查询
		auto it = _form_map.find(w.serialize());
		if (it != _form_map.end()) {
			return it->second;
		}
		// 失败返回-1
		return -1;
	}

	// 初始化
	void init() {
		_context_stk.clear();
		paras_count.clear();
		def_stk = 0;
		_is_def_blk = false;
		_def_para = 0;
		word_stk.clear();
		// 初始化临时工具
		_temp_tool.init();
	}
};

// 存放编译结果
class Compile_result {
protected:
	std::map<size_t, block_ptr> _sb_map; // id _> sb_map
	bool _has_blk;
public:
	Compile_result(): _has_blk(false){}
	~Compile_result() {}

	// 遍历所有的map
	void for_each_block(std::function<void (block_ptr&)> func) {
		for (auto& it: _sb_map) {
			func(it.second);
		}
	}

	// 初始化
	void init() {
		_sb_map.clear();
		_has_blk = false;
	}

	// 是否含有block
	inline bool has_block() const {	return _has_blk; }

	// 加入block
	void add_block(block_ptr block) {
		_has_blk = true;
		auto pair = std::make_pair(block->id(), block);
		_sb_map.insert(pair);
	}

	// 索引block
	block_ptr get_block_ref(size_t block_index) {
		assert(!_sb_map.empty());
		auto it = _sb_map.find(block_index);
		assert(it != _sb_map.end());
		auto& block = it->second;
		return block;
	}

	// 返回生成的block表
	std::map<size_t, block_ptr>& get_sb_map() {
		return _sb_map;
	}

	// 返回block的指令
	std::vector<Command> getCommandVector(size_t block_index) {
		auto it = _sb_map.find(block_index);
		assert(it != _sb_map.end());
		block_ptr block = get_block_ref(block_index);
		return block->instruct();
	}

	std::vector<Command>& refCommandVector(size_t block_index) {
		auto it = _sb_map.find(block_index);
		assert(it != _sb_map.end());
		block_ptr block = get_block_ref(block_index);
		return block->instruct();
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

// 前缀表达式 编译器
class S_Expr_Compiler: public Basic_Compiler
{
public:
	enum {
		// 分配给标识符的索引最大量
		max_slot_size = MAX_SLOT_SIZE,
		// 分配给block的最大数量
		max_block_index_size = MAX_BLOCK_INDEX_SIZE
	};

protected:
	std::vector<word_type_map> wt_map_list;		// 存放变量类型
	std::vector<local_index_map> li_map_list;	// 存放标识符到局部变量表的索引映射
	std::vector<_compiler_tool> ctool_stk;		// 编译工具栈， 进入时block入栈, 只对尾部元素操作, 退出block时出栈
	
	size_t _block_index = 0;					// 分配block的id用的
	std::vector<int> _vsblock_index_vec;		// block id栈

	Compile_result* _cresult_ptr;				// 存放结果
protected:

	// 新建一个block
	void new_block(Compile_result& result, size_t block_id) {
#if CHECK_Compiler_Field_NEW_BLK
		std::cout << "new_block" << std::endl;
#endif
		// 查看当前的ctool是否是strongField
		auto stronghold = ctool().strong_hold();
		block_ptr blk(new vsblock_static(block_id, stronghold));
		result.add_block(blk);

#if CHECK_Compiler_Field_NEW_BLK
		std::cerr << __LINE__ << " BLOCK_Field: " << (stronghold ? "T" : "F") << std::endl;
		// 检查所有的ctool
		for (_compiler_tool& ct : ctool_stk) {
			std::cout << "[";
			std::cout << "cur _strong_hold: " << ct._strong_hold << std::endl;
			std::cout << "temp _strong_hold: " << ct._temp_tool.subFieldStrongHold() << std::endl;
			std::cout << "]";
		}
		std::cout << std::endl << std::endl;
#endif
	}

	// 分配形参的地址给下一层
	int _auto_allc_form_para_index(Word& w) {
		return ctool()._temp_tool.add_para(w);
	}

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

	// 当前代码地址(top的block的代码地址), 如果没有生成过代码就返回0
	size_t _cur_comm_pos(Compile_result& result) {
		if (!result.has_block())return 0;
		else {
			size_t block_id = _cur_block_id();
			return result.get_block_ref(block_id)->instruct().size();
		}
	}

	// 分配给block的index
	size_t _alloc_block_index() {
		size_t index = ++_block_index;
		assert(max_block_index_size >= index);
		return index;
	}

	// 当前所在的block
	size_t _cur_block_id() {
		assert(!_vsblock_index_vec.empty());
		return _vsblock_index_vec.back();
	}

	// 当前所在的block
	block_ptr _cur_block_ptr() {
		assert(_cresult_ptr != nullptr);
		return _cresult_ptr->get_block_ref(_cur_block_id());
	}

public:
	// 生成list_block的代码
	virtual void generate_code(const std::vector<Word>& _word_vector, Compile_result& result, Context_helper& helper)
		throw (Context_error, undefined_exception);

	// 当前的ctool
	inline _compiler_tool& ctool() {
		assert(!ctool_stk.empty());
		return ctool_stk.back();
	}
	
	// 判断是否解引用
	void in_def() { ctool().in_def(); }
	void out_def() { ctool().out_def(); }

	// 判断是否生成call_blk语句
	void dont_gene_callblk() { ctool().in_def_blk(); }
	void enable_gene_callblk() { ctool().out_def_blk(); }

	// 判断IDENTIFIER_ENABLED word是否是形参声明
	void def_paras_begin() {
		ctool().def_paras_begin();
	}
	void def_paras_end() {
		ctool().def_paras_end();
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
#if CHECK_Compiler_Field
		std::cout << "localBegin" << std::endl;
#endif
		wt_map_list.push_back(word_type_map());
		li_map_list.push_back(local_index_map());

		// 新建一个ctool
		_compiler_tool _t_ctool;
		// 传递当前信息(第一次不传递)
		if (!ctool_stk.empty()) {
			auto& _temp_tool_ref = ctool().get_temp_tool_ref();
			_t_ctool.pass_temp_message(_temp_tool_ref);
		}
		ctool_stk.push_back(_t_ctool);
#if CHECK_Compiler_Field
		std::cout << __LINE__ << " 传递当前信息" << std::endl;
		for (_compiler_tool& ct : ctool_stk) {
			std::cout << "[";
			std::cout << "cur _strong_hold: " << ct._strong_hold << std::endl;
			std::cout << "temp _strong_hold: " << ct._temp_tool.subFieldStrongHold() << std::endl;
			std::cout << "]";
		}
		std::cout << std::endl << std::endl;
#endif
	}

	void localEnd() {
		wt_map_list.pop_back();
		li_map_list.pop_back();
		ctool_stk.pop_back();
#if CHECK_Compiler_Field
		std::cout << __LINE__ << " 退出作用域" << std::endl;
		for (_compiler_tool& ct : ctool_stk) {
			std::cout << "[";
			std::cout << "cur _strong_hold: " << ct._strong_hold << std::endl;
			std::cout << "temp _strong_hold: " << ct._temp_tool.subFieldStrongHold() << std::endl;
			std::cout << "]";
		}
		std::cout << std::endl << std::endl;
#endif
	}

	// 设置当前的作用域的子作用域的属性, 属性将会传递给下一个开辟的block
	void setSubFieldStrongHold() { 
		ctool().get_temp_tool_ref().setSubFieldStrongHold(true); 
#if CHECK_Compiler_Field
		std::cout << __LINE__ << " 前的作用域的子作用域为强作用域" << std::endl;
		for (_compiler_tool& ct : ctool_stk) {
			std::cout << "[";
			std::cout << "cur _strong_hold: " << ct._strong_hold << std::endl;
			std::cout << "temp _strong_hold: " << ct._temp_tool.subFieldStrongHold() << std::endl;
			std::cout << "]";
		}
		std::cout << std::endl << std::endl;
#endif
	}

	// 
	void setSubFieldWeakHold() {
		ctool().get_temp_tool_ref().setSubFieldStrongHold(false);
#if CHECK_Compiler_Field
		std::cout << __LINE__ << " 前的作用域的子作用域为弱作用域" << std::endl;
		for (_compiler_tool& ct : ctool_stk) {
			std::cout << "[";
			std::cout << "cur _strong_hold: " << ct._strong_hold << std::endl;
			std::cout << "temp _strong_hold: " << ct._temp_tool.subFieldStrongHold() << std::endl;
			std::cout << "]";
		}
		std::cout << std::endl << std::endl;
#endif
	}

	// 插入局部变量记录
	int insert_local(Word& w, WordType type) {
		wt_map_list.back().insert(
			std::make_pair(w.serialize(), type)
		);
		// 分配局部变量表的下标
		return _auto_alloc_local_index(w);
	}

	// 查看word是否被分配过局部变量下标, 否返回-1, 是返回下标大小
	int get_alloced_index(Word& w) {
		auto rbegin = li_map_list.rbegin();
		auto rend = li_map_list.rend();
		auto serialized_str = w.serialize();
		for (auto it = rbegin;it != rend; ++it) {
			auto fit = it->find(serialized_str);
			if (fit != it->end()) {
				// 存在这个下标
				size_t index = fit->second;
#if CHECK_Compiler_alloc
				std::cerr << "ALLOC index = " << index << std::endl;
#endif
				return index;
			}
			// 强作用域下不会向上查询
			else if(_cur_block_ptr()->strong_hold()){
				return -1;
			}
		}
		return -1;
	}

	// 查看word是否被分配过形参变量下标, 否返回-1, 是返回下标大小
	int get_form_para_alloced_index(Word& w) {
		/*auto rbegin = ctool_stk.rbegin();
		auto rend = ctool_stk.rend();
		for (auto it = rbegin; it != rend; ++it) {
			auto find_index = it->indexof_form_para(w);
			if (find_index != -1) {
				// 存在这个下标
#if CHECK_Compiler_alloc
				std::cerr << "ALLOC index = " << find_index << std::endl;
#endif
				return find_index;
			}
			// 继续向上查询
		}*/

		// 只找寻一层
		auto find_index = ctool().indexof_form_para(w);
		if (find_index != -1) {
			return find_index;
		}
		return -1;
	}

	// 判断单词的类型
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
			// 强作用域下不会向上查询
			else if (_cur_block_ptr()->strong_hold()) {
				return -1;
			}
		}
		return false;	// 没有这个变量名
	}

	// 初始化
	void init() {
		_block_index = 0;
		// ctool().init();
		// assert(_vsblock_index_vec.empty());
		// assert(ctool_stk.empty());
		
		_vsblock_index_vec.clear();
		ctool_stk.clear();
		wt_map_list.clear();
		//li_map_list.clear();
	}

	// 
	S_Expr_Compiler();
	virtual ~S_Expr_Compiler();
};