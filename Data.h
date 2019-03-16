#pragma once
#include <iostream>

#define CHECK_Data false

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

#if CHECK_Data
	virtual ~Data()
	{
		std::cout << "~Data " << toEchoString() << std::endl;
	}
#else
	virtual ~Data() = default;
#endif

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