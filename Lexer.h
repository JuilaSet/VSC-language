#pragma once

#include <memory>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <regex>

#define CHECK_Lexer false // 是否显示调试信息

const std::string EOS = "\0";
const std::regex pattern_NUMBER("^(\\+|-)?[0-9]+(\\.[0-9]+)?$");
const std::regex pattern_STRING_OPEN("^<$");
const std::regex pattern_STRING_CLOSED("^>$");
const std::regex pattern_LOCAL_OPEN("^\\[$");		// 定义局部变量栈_开始
const std::regex pattern_LOCAL_CLOSED("^\\]$");		// 定义局部变量栈_结束
const std::regex pattern_CONTEXT_CLOSED("^;$");		// 关闭上下文, 一句话结束
const std::regex pattern_COMMET_OPEN("^/\\*$");		// 定义注释_开始
const std::regex pattern_COMMET_CLOSED("^\\*/$");	// 定义注释_结束
const std::regex pattern_COMMET_LINE("^//$");		// 注释, 一行
const std::regex pattern_IDENTIFIER_ENABLED("^[\\_a-zA-Z]+[_a-zA-Z0-9]*$");	// 可作为标识符
const std::regex pattern_IDENTIFIER_SPEC("^\\$+[_a-zA-Z0-9]+$");			// 可作为特殊标识符

enum class IsToken : bool {
	FALSE = false, TRUE = true
};

// type相关: 是固定不变的, 因此不需要抽象为类

enum class WordType : int {
	EOS = -2,			// 结束符, 只有一个
	EMPTY = -1, 
	UNKOWN = 0, 
	SEPARATOR,			// 分隔符
	STRING,				// 字符串
	NUMBER,				// 数字
	LOCAL_OPEN,			// 局部变量开始
	LOCAL_CLOSED,		// 局部变量结束
	CONTEXT_CLOSED,		// 关闭上下文, 一句话结束
	STRING_OPEN,		// 字符串 |<
	STRING_CLOSED,		// >|
	COMMET_OPEN,		// 注释(会忽略之后的文本)
	COMMET_CLOSED,		// 注释(会忽略之后的文本)
	COMMET_LINE,		// 注释(会忽略之后的文本)
	IDENTIFIER_ENABLED,	// 可作为标识符(未注册的符号)
	IDENTIFIER,			// 正式标识符(注册完成的符号)
	IDENTIFIER_SPEC,	// 特殊标识符(代表一定数值的符号)
	OPERATOR_WORD,		// 运算符
	CONTROLLER,			// 流程控制符
	FUNC_PARA			// 函数形参
};

static std::string getWordTypeName(WordType type) {
	std::string ret = "";
	switch (type) {
	case WordType::EOS:
		ret = "EOS";
		break;
	case WordType::EMPTY:
		ret = "EMPTY";
		break;
	case WordType::UNKOWN:
		ret = "UNKOWN";
		break;
	case WordType::SEPARATOR:
		ret = "SEPARATOR";
		break;
	case WordType::STRING:
		ret = "STRING";
		break;
	case WordType::NUMBER:
		ret = "NUMBER";
		break;
	case WordType::LOCAL_OPEN:
		ret = "LOCAL_OPEN";
		break;
	case WordType::LOCAL_CLOSED:
		ret = "LOCAL_CLOSED";
		break;
	case WordType::STRING_OPEN:
		ret = "STRING_OPEN";
		break;
	case WordType::STRING_CLOSED:
		ret = "STRING_CLOSED";
		break;
	case WordType::COMMET_OPEN:
		ret = "COMMET_OPEN";
		break;
	case WordType::COMMET_CLOSED:
		ret = "COMMET_CLOSED";
		break;
	case WordType::COMMET_LINE:
		ret = "COMMET_LINE";
		break;
	case WordType::OPERATOR_WORD:
		ret = "KEY_WORD";
		break;
	case WordType::IDENTIFIER_ENABLED:
		ret = "IDENTIFIER_ENABLED";
		break;
	case WordType::IDENTIFIER:
		ret = "IDENTIFIER";
		break;
	case WordType::IDENTIFIER_SPEC:
		ret = "IDENTIFIER_SPEC";
		break;
	case WordType::CONTEXT_CLOSED:
		ret = "CONTEXT_CLOSED";
		break;
	case WordType::CONTROLLER:
		ret = "CONTROLLER";
		break;
	case WordType::FUNC_PARA:
		ret = "FUNC_PARA";
		break;
	default:
		break;
	}
	return ret; 
}

// word

class Word {
protected:
	WordType _type_;
	std::string _str_;
public:
	Word(WordType type = WordType::EMPTY, std::string s = "") :_type_(type), _str_(s) {};
	WordType getType() { return _type_; };
	std::string getString() { return _str_; }

	// 序列化word
	std::string serialize() { return _str_ + "_" + getWordTypeName(_type_); }

	data_ptr getData() const {
		if (_type_ == WordType::NUMBER) {
			int res;
			std::stringstream ss;
			ss << _str_;
			ss >> res;
			return data_ptr(new NumData(DataType::NUMBER, res));
		}
		else {
			return data_ptr(new StringData(_str_));
		}
	}
};

class WordTypeHelper {
	friend class WORD_TYPE_HELPER;
protected:
	std::set<std::string> _operators_set;
	std::set<std::string> _controller_set;
public:
	WordTypeHelper() = default;
	void regist_operator(const std::string str);	// 注册关键字
	void regist_controller(const std::string str);	// 注册流程控制符
	WordType calc_word_type(const std::string& str, IsToken b) const;
};

#define REGIST_OPERATO_WORDS(word_type_helper, str) \
word_type_helper.regist_operator(str); \

//

#define REGIST_CONTROLLER_WORDS(word_type_helper, str) \
word_type_helper.regist_controller(str); \

//

class Input;
class Lexer
{
	unsigned int _fd;
	bool has_read;
	std::vector<Word> _words_list;
	std::unique_ptr<Input> _input;
	bool _flag_ignore = false;	// 遇到注释就忽略里面所有的内容
	bool _flag_ignore_line = false;	// 遇到行注释就忽略里面所有的内容直到行位
	bool _flag_string_begin = false;	// 遇到行注释就忽略里面所有的内容直到行位
	std::string _string_;

#if CHECK_Lexer
	std::stringstream ofs;
#endif

protected:
	bool _lexLine(Token_helper& helper, WordTypeHelper& word_type_helper);	// 分割n行单词, 返回文件是否读完
	bool _push_into_list(std::string str, IsToken b, const WordTypeHelper& word_type_helper) ;
	inline std::string getChar(std::string&, unsigned int& str_p);
public:

	Lexer();
	Lexer(Input* input);

	bool fillList(Token_helper& tokenHelper, WordTypeHelper& word_type_helper, bool fillAll=true, unsigned int size=0);	// 填充列表size行单词 返回文件是否读完, 填入NO_RANGE表示读取全部文件内容

	std::vector<Word>& get_words_list() {
		return _words_list;
	}

	bool next();	// 返回是否还有东西可读
	Word read();
	Word peek();
	
	Word getWord(unsigned int n);
	void seek(unsigned int n); // 移动指针到某个位置
	void gotoBegin();
	void init();

#if CHECK_Lexer
	void log_test(const std::string& name);
#endif

	virtual ~Lexer();
};
