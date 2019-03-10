#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <cassert>

const std::string EOL = "\n";
const std::string CONLIN = "\\";

class Input {
protected:
	bool _isEnd;
public:
	virtual std::string getLine() = 0;
	bool isEnd() const {
		return _isEnd;
	}
};

class StringInput : public Input {
	std::string _str;
public:
	StringInput(std::string str) :_str(str) {};
	StringInput() :StringInput("") {}	// 空对象模式使用
	std::string getLine();
	void setString(std::string str) { _str = str; }
};

class CLIInput : public Input {
	std::string _lineHead;
public:
	CLIInput():_lineHead("-> ") {};
	CLIInput(std::string _head) :_lineHead(_head) {};
	std::string getLine();
};


class FileInput: public Input {
protected:
	bool _isOpened;
	std::fstream _ifile;
public:
	FileInput();
	FileInput(std::string filename) throw(std::string);
	void close();
	
	bool next();
	std::string read();
	virtual std::string getLine() override;

	// std::string peek();
	void seekg(int advance);

	// 辅助函数

	bool isOpened() const
	{
		return _isOpened;
	}

	virtual ~FileInput();
};