#pragma once

#include <string>
#include <map>
#include <memory>
// 符号类, 用于分割单词
class Token
{
protected:
	Token();
	Token(const std::string, const std::string name = "");

	const std::string name;
public:
	virtual operator bool();

	// 方法
	const std::string token;
	virtual bool check(const std::string&) const;
	virtual bool operator==(const Token&) const;
	virtual bool operator<(const Token&) const;
	
	std::string get_name() const;
	std::string get_token() const;

	virtual ~Token();

	// 友元类
	friend class Token_helper;
};

class TOKEN_HELPER;
class Token_helper
{
	std::map<const std::string, Token> _token_map;
	std::multimap<const char, Token> _first_sign_map; // 关联符号字符串第一个char可能是哪些符号_
	using _mutimap_char_token_iter = std::multimap<const char, Token>::iterator;

public:
	Token_helper() = default;
	~Token_helper() = default;

	bool isToken(const std::string& token_str);
	bool isToken_char(const char c);

	void regist_token(const std::string& token_str, const std::string name = "");
	std::vector<Token> get_all_tokens();
	Token get_token_obj(const std::string& token_str);
	std::pair<_mutimap_char_token_iter, _mutimap_char_token_iter> get_token_range(const char c);
};
/* []][
class TOKEN_HELPER {
public:
	static Token_helper token_helper;	// 单例对象
};
*/
//
// 用于注册符号的宏
//
#define REGIST_TOKEN(HELPER, TOKEN_SIGN, TOKEN_NAME)	\
HELPER.regist_token(TOKEN_SIGN, TOKEN_NAME);			\

//