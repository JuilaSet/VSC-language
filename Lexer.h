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
const std::regex pattern_IDENTIFIER_ENABLED("^[_a-zA-Z]+[_a-zA-Z0-9]*$");	// 可作为标识符
const std::regex pattern_IDENTIFIER("^\\$[_a-zA-Z0-9]+$");			// 标识符

enum class DataType :int {
	// 空, enum默认构造为NON
	NON,
	// 字符串(只读转换为number)
	STRING,
	// 整型(只读转换为string)
	NUMBER,
	// opcode地址型, 不允许转换为任何类型
	OPERA_ADDR
};

// Data只能是堆对象
struct data_t
{
	std::string value_str;
	int value_int;
};

class Data {
protected:
	DataType type;
	data_t _data;
public:
	Data() :type(DataType::NON), _data({ "", 0 }) {}

	Data(DataType type, int data) :type(type) {
		_data.value_int = data;
	}

	Data(DataType type, const std::string data) :type(type) {
		_data.value_str = data;
	}

	Data(const Data& d) {
		type = d.type;
		if (type == DataType::STRING) {
			_data.value_str = d._data.value_str;
		}
		else {
			_data.value_int = d._data.value_int;
		}
	}

	virtual ~Data() = default;

	Data operator=(const Data& d) {
		type = d.type;
		if (d.type == DataType::STRING) {
			_data.value_str = d._data.value_str;
		}
		else {
			_data.value_int = d._data.value_int;
		}
		return *this;
	}

	bool operator==(const Data& d) {
		bool ret = false;
		if (d.type == type) {
			if (type == DataType::STRING) {
				ret = _data.value_str == d._data.value_str;
			}
			else {
				ret = _data.value_int == d._data.value_int;
			}
		}
		else {
			ret = toString() == d.toString();
		}
		return ret;
	}

	bool operator < (const Data& d) {
		bool ret = false;
		if (d.type == type) {
			if (type == DataType::STRING) {
				ret = _data.value_str < d._data.value_str;
			}
			else {
				ret = _data.value_int < d._data.value_int;
			}
		}
		return ret;
	}

	bool operator > (const Data& d) {
		bool ret = false;
		if (d.type == type) {
			if (type == DataType::STRING) {
				ret = _data.value_str > d._data.value_str;
			}
			else {
				ret = _data.value_int > d._data.value_int;
			}
		}
		return ret;
	}

	// 回显用函数
	std::string toEchoString() const {
		if (type == DataType::STRING) {
			return "<" + _data.value_str + ">";
}
		else {
			std::stringstream ss;
			ss << _data.value_int;
			return ss.str();
		}
	}

	// 返回转换的字符串(支持数字转换为字符串)
	virtual std::string toString() const {
		if (type == DataType::STRING) {
			return _data.value_str;
		}
		else {
			assert(type != DataType::OPERA_ADDR);
			std::stringstream ss;
			ss << _data.value_int;
			return ss.str();
		}
	}

	// 返回转换的bool型(支持数字, 字符串转换bool)
	bool toBool() const {
		bool ret;
		switch (type)
		{
		case DataType::NON:
			ret = false;
			break;
		case DataType::STRING:
			ret = _data.value_str == "" ? false : true;
			break;
		case DataType::NUMBER:
			ret = _data.value_int == 0 ? false : true;
			break;
		case DataType::OPERA_ADDR:
			assert(false);
			break;
		default:
			ret = false;
			break;
		}
		return ret;
	}
	
	// 返回转换的数字(支持字符串转换为数字)
	int toNumber() const {
		// 字符串转换为数字
		if (type == DataType::STRING) {
			std::stringstream s;
			int a;
			s << _data.value_str;
			s >> a;
			return a;
		}
		else {
			assert(type != DataType::OPERA_ADDR);	// 不允许用户自定义跳转位置
			return _data.value_int;
		}
	}

	// 返回地址(只能是地址类型)
	unsigned int toAddr() const {
		assert(type == DataType::OPERA_ADDR);
		return _data.value_int;
	}

	DataType getType() const {
		return this->type;
	}

	std::string toTypeName() const {
		switch (type) {
		case DataType::NON:
			return "NON";
		case DataType::STRING:
			return "STRING";
		case DataType::NUMBER:
			return "NUMBER";
		case DataType::OPERA_ADDR:
			return "OPERA_ADDR";
		default:
			assert(false);
			return "ERROR";
		}
	}
};

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
	OPERATOR_WORD,		// 运算符
	CONTROLLER			// 流程控制符
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
	case WordType::CONTEXT_CLOSED:
		ret = "COMMET_CLOSED";
		break;
	case WordType::CONTROLLER:
		ret = "CONTROLLER";
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

	Data getData() const {
		if (_type_ == WordType::NUMBER) {
			int res;
			std::stringstream ss;
			ss << _str_;
			ss >> res;
			return Data(DataType::NUMBER, res);
		}
		else {
			return Data(DataType::STRING, _str_);
		}
	}
};

class WordTypeHelper {
	friend class WORD_TYPE_HELPER;
protected:
	WordTypeHelper() = default;
	std::set<std::string> _operators_set;
	std::set<std::string> _controller_set;
public:
	void regist_operator(const std::string str);	// 注册关键字
	void regist_controller(const std::string str);	// 注册流程控制符
	WordType calc_word_type(std::string& str, IsToken b);
}; 

class WORD_TYPE_HELPER {
public:
	static WordTypeHelper word_type_helper;	// 单例对象
};

#define REGIST_OPERATO_WORDS(str) \
WORD_TYPE_HELPER::word_type_helper.regist_operator(str); \

//

#define REGIST_CONTROLLER_WORDS(str) \
WORD_TYPE_HELPER::word_type_helper.regist_controller(str); \

//

class FileInput;
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
	bool _lexLine();	// 分割n行单词, 返回文件是否读完
	bool _push_into_list(std::string str, IsToken b);
	inline std::string getChar(std::string&, unsigned int& str_p);
public:

	Lexer();
	Lexer(Input* input);

	bool fillList(bool fillAll=true, unsigned int size=0);	// 填充列表size行单词 返回文件是否读完, 填入NO_RANGE表示读取全部文件内容

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
