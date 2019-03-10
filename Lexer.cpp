#include "pch.h"
#include "Lexer.h"
#include "Input.h"
#include "Token.h"
#define REGEX_MATCH(PATTERN) \
if (std::regex_search(str, result, pattern_##PATTERN)) {	\
	type = WordType::PATTERN;								\
}\
//

WordType WordTypeHelper::calc_word_type(std::string& str, IsToken b)
{
	// key_words
	if (_operators_set.find(str) != _operators_set.end())
	{
		return WordType::OPERATOR_WORD;
	}
	else if (_controller_set.find(str) != _controller_set.end()) {
		return WordType::CONTROLLER;
	}
	// others
	WordType type = b == IsToken::TRUE ? WordType::SEPARATOR : WordType::UNKOWN;
	std::smatch result;
	int strsize = str.size();
	// 
	REGEX_MATCH(NUMBER);
	REGEX_MATCH(STRING_OPEN);
	REGEX_MATCH(STRING_CLOSED);
	REGEX_MATCH(LOCAL_OPEN);
	REGEX_MATCH(LOCAL_CLOSED);
	REGEX_MATCH(CONTEXT_CLOSED);
	REGEX_MATCH(COMMET_OPEN);
	REGEX_MATCH(COMMET_CLOSED);
	REGEX_MATCH(COMMET_LINE);
	REGEX_MATCH(IDENTIFIER_ENABLED);
	return type;
}

void WordTypeHelper::regist_operator(const std::string str) {
	_operators_set.insert(str);
}

void WordTypeHelper::regist_controller(const std::string str) {
	_controller_set.insert(str);
}

WordTypeHelper WORD_TYPE_HELPER::word_type_helper = WordTypeHelper();

// Lexer

Lexer::Lexer(Input* input) : Lexer()
{
	_input.reset(input);

#if CHECK_Lexer
	ofs = std::stringstream();
#endif

}

Lexer::Lexer() : _fd(0), has_read(false)
{
	REGIST_TOKEN("/*", commet_open);
	REGIST_TOKEN("*/", commet_close);
	REGIST_TOKEN("|<", cstring_open);
	REGIST_TOKEN(">|", cstring_closed);
	REGIST_TOKEN("//", commetLine);
}

// 只分析一行用
bool Lexer::_push_into_list(std::string str, IsToken b)
{
	if (str != "")
	{
		WordType type = WORD_TYPE_HELPER::word_type_helper.calc_word_type(str, b);
		// commet
		if (!_flag_string_begin&& !_flag_ignore && type == WordType::COMMET_LINE)_flag_ignore_line = true;
		if (!_flag_ignore_line) {
			if (!_flag_string_begin && type == WordType::COMMET_OPEN)_flag_ignore = true;
			if (!_flag_ignore) {
				if (!has_read) has_read = true;	// 只要push过一次就说明读取过
				if (type == WordType::STRING_CLOSED) {
					_words_list.emplace_back(WordType::STRING, _string_);
					_flag_string_begin = false;
					_string_ = "";
				}
				// push
				if (_flag_string_begin)
					_string_ += str;
				else if(type != WordType::SEPARATOR)	// 不会加入分隔符, 除非是字符串
					_words_list.emplace_back(type, str);
				if (type == WordType::STRING_OPEN)_flag_string_begin = true;
			}
			if (!_flag_string_begin && type == WordType::COMMET_CLOSED)_flag_ignore = false;
		}
#if CHECK_Lexer
		std::cerr << __LINE__ << ": push_back = " << str << std::endl;
		ofs << __LINE__ << ": push_back = " << str << std::endl;
#endif
		return true;
	}
	return false;
}

std::string Lexer::getChar(std::string& line, unsigned int& str_p)
{
#if CHECK_lexer
	std::cerr << __LINE__ << ": get char from line = " << line.substr(0, line.size() - 1) << ", p = " << str_p << std::endl;
	ofs << __LINE__ << ": get char from line = " << line.substr(0, line.size() - 1) << ", p = " << str_p << std::endl;
#endif
	assert(line.size() > str_p);
	std::string s;
	s = line[str_p++];
	return s;
}

bool Lexer::_lexLine()
{
	auto& tokenHelper = TOKEN_HELPER::token_helper;
	auto tokens_set = tokenHelper.get_all_tokens();
	std::string line = _input->getLine();

	unsigned int str_p = 0;	// 字符串当前指针
	std::string word = "";
	while(true)
	{
		std::string nextChar = getChar(line, str_p);

#if CHECK_Lexer
		std::cerr << __LINE__ << ": nextChar = " << nextChar << std::endl;
		ofs << __LINE__ << ": nextChar = " << nextChar << std::endl;
#endif

		if (nextChar == EOL)
		{
			_push_into_list(word, IsToken::FALSE);
			goto finish;
		}

#if CHECK_Lexer
		std::cerr << __LINE__ << ": nextChar[0] = " << nextChar[0] << std::endl;
		ofs << __LINE__ << ": nextChar[0] = " << nextChar[0] << std::endl;
#endif

		if (tokenHelper.isToken_char(nextChar[0]))
		{
			int maxLen = 0;
			auto range = tokenHelper.get_token_range(nextChar[0]);
			auto begin = range.first, end = range.second;
			for (auto it = begin; it != end; ++it)
			{
				int size = it->second.get_token().size();
#if CHECK_Lexer
				std::cerr << __LINE__ << ": \tmaybe = " << it->second.get_token() << std::endl;
				ofs << __LINE__ << ": \tmaybe = " << it->second.get_token() << std::endl;
#endif
				if (maxLen < size)
				{
					maxLen = size;
				}
			}	// 找到 maxLen

#if CHECK_Lexer
			std::cerr << __LINE__ << ": \tmaxLen = " << maxLen << std::endl;
			ofs << __LINE__ << ": \tmaxLen = " << maxLen << std::endl;
#endif
			int curp = str_p - 1;		// 保存起点
			int endp = curp + maxLen;	// 终点
			std::string tokenFound = "";
			std::string tempStr = nextChar.substr(0, 1);
#if CHECK_Lexer
			std::cerr << __LINE__ << ": tempStr = " << tempStr << std::endl;
			std::cerr << __LINE__ << ": For = " << curp << " to " << endp << std::endl;
			ofs << __LINE__ << ": tempStr = " << tempStr << std::endl;
			ofs << __LINE__ << ": For = " << curp << " to " << endp << std::endl;
#endif
			std::string c;
			while(true)
			{
#if CHECK_Lexer
				std::cerr << __LINE__ << ": \ttempStr = " << tempStr << std::endl;
				ofs << __LINE__ << ": \ttempStr = " << tempStr << std::endl;
#endif
				if (tokenHelper.isToken(tempStr))
				{
					tokenFound = tempStr;
#if CHECK_Lexer
					std::cerr << __LINE__ << ": \tmatched = " << tokenFound << std::endl;
					ofs << __LINE__ << ": \tmatched = " << tokenFound << std::endl;
#endif
				}
				if (str_p == endp)break;

				c = getChar(line, str_p);
				if (c == EOL)
				{
					if (tokenFound != "")	// 找到
					{
						_push_into_list(word, IsToken::FALSE);
						_push_into_list(tokenFound, IsToken::TRUE);
					}
					goto finish;	// 行读取完毕
				}
				tempStr += c;
#if CHECK_Lexer
				std::cerr << __LINE__ << ": \tstr_p = " << str_p << std::endl;
				ofs << __LINE__ << ": \tstr_p = " << str_p << std::endl;
#endif
			}	// find finish

			if (tokenFound != "")	// 找到
			{
#if CHECK_Lexer
				std::cerr << __LINE__ << ": Found Success !" << std::endl;
				std::cerr << __LINE__ << ": \ttokenFound = " << tokenFound << std::endl;
				ofs << __LINE__ << ": Found Success !" << std::endl;
				ofs << __LINE__ << ": \ttokenFound = " << tokenFound << std::endl;
#endif
				_push_into_list(word, IsToken::FALSE);
				word = "";
				_push_into_list(tokenFound, IsToken::TRUE);
				str_p = curp + tokenFound.size();
			}
			else
			{
				word += nextChar;
				str_p = curp + 1;
			}
		}
		else
		{
			word += nextChar;
		}
	}

// unfinish:
#if CHECK_lexer
	std::cerr << __LINE__ << ": lexer unFinish" << std::endl;
	ofs << __LINE__ << ": lexer unFinish" << std::endl;
#endif
	_flag_ignore_line = false;	// 一行结束, 行注释flag置为真
	return false;

finish:
	_flag_ignore_line = false;	// 一行结束, 行注释flag置为真
	// 保证之后的string都被放入列表
	if(_flag_string_begin)
		_words_list.emplace_back(WordType::STRING, _string_);
	_push_into_list(EOL, IsToken::TRUE);	// 一行结束换行

#if CHECK_lexer
	std::cerr << __LINE__ << ": lexer Finish" << std::endl;
	ofs << __LINE__ << ": lexer Finish" << std::endl;
#endif
	return true;
}

void Lexer::init() {
	_flag_ignore = false;		// 遇到注释就忽略里面所有的内容
	_flag_ignore_line = false;	// 遇到行注释就忽略里面所有的内容直到行位
	_flag_string_begin = false;	// 遇到行注释就忽略里面所有的内容直到行位
	_fd = 0;
	has_read = false;
	_words_list.clear();
}

bool Lexer::fillList(bool fillAll, unsigned int size)
{
	_string_ = "";
	bool flag = false;
	for (unsigned int i = 0; (fillAll || i < size); ++i)
	{
		flag = _lexLine();
		if (_input->isEnd()) {
			// 结束符放入列表
			_words_list.push_back(Word(WordType::EOS, EOS));
#if CHECK_Lexer
			std::cerr << __LINE__ << ": push_back = EOS" << std::endl;
			ofs << __LINE__ << ": push_back = EOS" << std::endl;
#endif
			break;
		}
	}
	return flag;
}

bool Lexer::next()
{
	if (!has_read) return false;	// 如果还没有读入内容, 返回false
	unsigned int s = _words_list.size();
	if (s == 0) return false;	// 内容为空, 返回

	if (_fd + 1 == _words_list.size())return false;	// 在结尾保持不变
	++_fd;
	return true;
}

Word Lexer::read()
{
	if (!has_read) return Word();	// 如果还没有读入内容, 返回""
	
	unsigned int s = _words_list.size();
	if (s  == 0) return Word();	// 内容为空, 返回""

	if (_fd + 1 == s) ;	// 在结尾保持不变
	else ++_fd;

	return _words_list[_fd];
}

Word Lexer::peek()
{
	if (!has_read) return Word();	// 如果还没有读入内容, 返回""
	unsigned int s = _words_list.size();
	if (s == 0) return Word();	// 内容为空, 返回""
	
	assert(_fd < s);
	return _words_list.at(_fd);
}

Word Lexer::getWord(unsigned int n)
{
	assert(n < _words_list.size());
	return _words_list[n];
}

// 移动指针到某个位置
void Lexer::seek(unsigned int n) 
{
	assert(n < _words_list.size());
	_fd = n;
}

void Lexer::gotoBegin()
{
	_fd = 0;
}

#if CHECK_lexer
	void Lexer::log_test(const std::string& name)
	{
		ofs << std::endl << "\n_words_list:" << std::endl;
		for (auto word : _words_list)
		{
			ofs << "[" << getWordTypeName(word.getType()) << "- " << word.getString() << "], ";
		}

		std::ofstream of(name);
		of << ofs.str();
	}
#endif

Lexer::~Lexer()
{
	// 析构自动调用 _input->close();
}