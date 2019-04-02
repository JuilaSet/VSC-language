#include "pch.h"
#include "Token.h"

// Token_helper::

bool Token_helper::isToken(const std::string& token_str)
{
	if (_token_map.count(token_str) > 0)
	{
		return true;
	}
	else
	{
		return false;
	}

}

void Token_helper::regist_token(const std::string& token_str, const std::string name)
{
	Token t(token_str, name);
	_token_map.insert(std::make_pair(token_str, t));
	_first_sign_map.insert(std::make_pair(token_str.at(0), t));
}

Token Token_helper::get_token_obj(const std::string& token_str)
{
	if (_token_map.count(token_str) > 0)
		return _token_map.find(token_str)->second;
	else
		return Token();
}

std::pair<Token_helper::_mutimap_char_token_iter, Token_helper::_mutimap_char_token_iter> 
		Token_helper::get_token_range(const char c) 
{
	return _first_sign_map.equal_range(c);
}

std::vector<Token> Token_helper::get_all_tokens()
{
	std::vector<Token> tokens;
	auto begin = _token_map.begin(), end = _token_map.end();
	for (auto it = begin; it != end; ++it)
	{
		tokens.push_back(it->second);
	}
	return tokens;
}

bool Token_helper::isToken_char(const char c)
{
	return _first_sign_map.count(c) > 0 ? true : false;
}

// Token::

Token::Token(const std::string str, const std::string name): token(str), name(name)
{
}

Token::Token():token("")
{
}

Token::operator bool()
{
	return token == "" ? false : true;
}
bool Token::check(const std::string& str) const
{
	return str == token ? true : false;
}

bool Token::operator==(const Token& another) const
{
	return another.token == token ? true : false;
}

bool Token::operator<(const Token& t) const
{
	return token < t.token;
}

std::string Token::get_name() const
{
	return name;
}

std::string Token::get_token() const
{
	return token;
}

Token::~Token()
{
}