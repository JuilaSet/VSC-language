#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <cassert>

const std::string EOL = "\n";
const std::string CONLIN = "\\";

// 分割字符串放入dest
void split(std::vector<std::string>& dest, std::string str, std::string pattern);

class Input {
protected:
	bool _isoverflow;
	int _readsize;
	bool _isEnd;
public:
	virtual std::string getLine() = 0;
	bool isEnd() const {
		return _isEnd;
	}
};

class StringInput : public Input {
	std::vector<std::string> lines;				// 存放每一行
	int i;										// 行地址
public:
	StringInput()	// 空对象模式使用
		:i(0){}	

	StringInput(std::string str): i(0) {
		// 放入每一行
		split(lines, str, EOL);
	}
	
	virtual std::string getLine() override;
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