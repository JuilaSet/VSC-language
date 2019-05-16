#include "pch.h"
#include "Input.h"

#define CHECK_input false

void split(std::vector<std::string>& dest, std::string str, std::string pattern) {
	std::string::size_type pos;
	std::vector<std::string> result;
	// 最后加上这个分隔符便于操作
	str += pattern;
	int size = str.size();
	for (int i = 0; i < size; i++) {
		// 从i位置开始寻找pattern字符串
		pos = str.find(pattern, i);
		if (pos < size) {
			// 不包括字符串
			std::string s = str.substr(i, pos - i);
			dest.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
}

std::string StringInput::getLine()
{
	if (i == lines.size()) {
		_isEnd = true;
		return EOL;
	}
	return lines[i++] + EOL;
}

std::string CLIInput::getLine()
{
	std::stringstream str;
	char c;
	bool eolflag = false;
	_isEnd = false;
	std::cout << _lineHead;
	while (1) {
		c = std::cin.get();
		if (!eolflag && c == EOL[0]) {
			_isEnd = true;
			break;	// 回车退出
		}
		eolflag = false;
		
		if (c == CONLIN[0]) eolflag = true;	// \号连接两行, 不会加入回车
		else if(c != EOL[0])str << c;		// 加入这个非回车字符
	}
#if CHECK_input
	std::cerr << str.str() << std::endl;
#endif
	return str.str() + EOL;
}

FileInput::FileInput()
{
	_isOpened = false;
	_isEnd = true;
}

FileInput::FileInput(std::string filename) throw(std::string) : _ifile(filename, std::ios_base::in)
{
	if (!_ifile) 
	{
		std::cerr << "open failed" << std::endl;
		throw "Error: open failed, file name = " + filename;
	}
	_isOpened = true;
	_isEnd = false;
}

void FileInput::close()
{
	// 检查文件是否关闭 std::cerr << "closed" << std::endl;
	assert(_isOpened); // 文件一定是打开的
	_isOpened = false;
	_ifile.close();
}

bool FileInput::next()
{
	char c = _ifile.get();
	if (c == EOF)
	{
		_isEnd = true;
		goto end;
	}
	if (c < 0)	// 中文
	{
		_ifile.seekg(2 * sizeof(char), std::ios_base::cur);
	}
	return true;
end:
	return false;
}

std::string FileInput::read()
{
	std::string str = "";
	char c = _ifile.get();
	if (c == EOF)
	{
		_isEnd = true;
		goto end;
	}
	str.push_back(c);
	if (c < 0)
	{
		c = _ifile.get();
		str.push_back(c);

		c = _ifile.get();
		str.push_back(c);
		goto finish;
	}

finish:
	return str;
end:
	return "";
}

std::string FileInput::getLine()
{
	std::string line;
	std::getline(_ifile, line);
	line += EOL;
	_isEnd = _ifile.eof();
	return line;
}
/*
std::string FileInput::peek()
{
	std::string str = "";
	char c = _ifile.peek();
	if (_isEnd)
	{
		goto end;
	}
	if(c == EOF)
	{
		goto end;
	}
	if (c < 0)  // 中文字符
	{
		char chs[3];
		auto ps = _ifile.tellg();
		_ifile.read(chs, 3 * sizeof(char));
		_ifile.seekg(0, ps);
		for (char ch : chs)
		{
			str.push_back(ch);
		}
		goto finish;
	}
	else 
	{
		str.push_back(c);
		goto finish;
	}

finish:
	return str;
end:
	return "";
}
*/
void FileInput::seekg(int advance)
{
	_ifile.seekg(advance, std::ios::cur);
}

FileInput::~FileInput()
{
	if (_isOpened) 
	{
		close();
	}
}
