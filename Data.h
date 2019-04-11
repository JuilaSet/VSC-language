#pragma once

#define CHECK_Data false

enum class DataType :int {
	// 空, enum默认构造为NON
	NON,
	// 字符串(只读转换为number)
	STRING,
	// 整型(只读转换为string)
	NUMBER,
	// opcode地址型, 不允许转换为任何类型
	OPERA_ADDR,
	// 索引类型, 只允许用于索引
	ID_INDEX,
	// block的索引类型
	BLK_INDEX,
	// 参数类型
	PARA_INDEX
};

class vsData {
protected:
	DataType _type;
public:
	vsData(DataType type) :_type(type) {}

	// 比较的方法

	virtual bool operator==(const vsData& d) = 0;

	virtual bool operator < (const vsData& d) = 0;

	virtual bool operator > (const vsData& d) = 0;

	// 回显用函数
	virtual std::string toEchoString() const = 0;

	// 返回转换的字符串(支持数字转换为字符串)
	virtual std::string toString() const = 0; 

	// 返回转换的bool型(支持数字, 字符串转换bool)
	virtual bool toBool() const = 0; 

	// 返回转换的数字(支持字符串转换为数字)
	virtual long long toNumber() const = 0;

	// 返回地址(只能是地址类型)
	virtual unsigned int toAddr() const = 0; 

	// 返回索引(只能是索引类型)
	virtual size_t toIndex() const = 0; 

	// 查询成员
	virtual std::shared_ptr<vsData> in(size_t index) = 0;

	// 返回类型
	DataType getType() const {
		return _type;
	}

	// 返回类型名称
	std::string getTypeName() const  {
		switch (_type) {
		case DataType::NON:
			return "NON";
		case DataType::STRING:
			return "STRING";
		case DataType::NUMBER:
			return "NUMBER";
		case DataType::OPERA_ADDR:
			return "OPERA_ADDR";
		case DataType::ID_INDEX:
			return "ID_INDEX";
		case DataType::BLK_INDEX:
			return "BLK_INDEX";
		case DataType::PARA_INDEX:
			return "PARA_INDEX";
		default:
			assert(false);
			return "ERROR";
		}
	}

}; 
using data_ptr = std::shared_ptr<vsData>;

class NumData : public vsData, public std::enable_shared_from_this<NumData> {
protected:
	size_t value;

public:
	NumData():vsData(DataType::NON), value(0) { }

	NumData(DataType type, int data) : vsData(type), value(data) { }

	NumData(const vsData& d) :vsData(d.getType()), value(d.toNumber()) { }

	NumData(const NumData& d) :vsData(d.getType()), value(d.value) { }

#if CHECK_Data
	virtual ~Data()
	{
		std::cout << "~Data " << toEchoString() << std::endl;
	}
#endif

	virtual void operator=(const NumData& d) {
		value = d.value;
	}

	// 比较
	virtual bool operator==(const vsData& d) override {
		bool ret = false;
		switch (d.getType())
		{
		case DataType::STRING:
			ret = value == d.toNumber(); // 与Number类型比较
			break;
		case DataType::NON:
		case DataType::NUMBER:
			ret = value == d.toNumber();
			break;
		case DataType::OPERA_ADDR:
			ret = value == d.toAddr();
			break;
		case DataType::BLK_INDEX:
		case DataType::ID_INDEX:
		case DataType::PARA_INDEX:
			ret = value == d.toIndex();
			break;
		default:
			break;
		}
		return ret;
	}

	virtual bool operator < (const vsData& d) override {
		bool ret = false;
		switch (d.getType())
		{
		case DataType::STRING:
			ret = value < d.toNumber(); // 与Number类型比较
			break;
		case DataType::NON:
		case DataType::NUMBER:
			ret = value < d.toNumber();
			break;
		case DataType::OPERA_ADDR:
			ret = value < d.toAddr();
			break;
		case DataType::BLK_INDEX:
		case DataType::ID_INDEX:
		case DataType::PARA_INDEX:
			ret = value < d.toIndex();
			break;
		default:
			break;
		}
		return ret;
	}

	virtual bool operator > (const vsData& d) override {
		bool ret = false;
		switch (d.getType())
		{
		case DataType::STRING:
			ret = value > d.toNumber(); // 与Number类型比较
			break;
		case DataType::NON:
		case DataType::NUMBER:
			ret = value > d.toNumber();
			break;
		case DataType::OPERA_ADDR:
			ret = value > d.toAddr();
			break;
		case DataType::BLK_INDEX:
		case DataType::ID_INDEX:
		case DataType::PARA_INDEX:
			ret = value > d.toIndex();
			break;
		default:
			break;
		}
		return ret;
	}

	// 回显用函数
	virtual std::string toEchoString() const override {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	// 返回转换的字符串(支持数字转换为字符串)
	virtual std::string toString() const override {
		assert(_type != DataType::OPERA_ADDR);
		assert(_type != DataType::ID_INDEX);
		assert(_type != DataType::BLK_INDEX);
		assert(_type != DataType::PARA_INDEX);
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	// 返回转换的bool型(支持数字, 字符串转换bool)
	virtual bool toBool() const override {
		return value;
	}

	// 返回转换的数字(支持字符串转换为数字)
	virtual long long toNumber() const override {
		assert(_type == DataType::NUMBER);
		return value;
	}

	// 返回地址(只能是地址类型)
	virtual unsigned int toAddr() const override {
		assert(_type == DataType::OPERA_ADDR);
		return value;
	}

	// 返回索引(只能是索引类型)
	virtual size_t toIndex() const override {
		assert(_type == DataType::ID_INDEX || _type == DataType::BLK_INDEX || _type == DataType::PARA_INDEX);
		return value;
	}
	
	// 查询成员(之后的成员全为0)
	virtual data_ptr in(size_t index) override {
		return data_ptr(new NumData(DataType::NUMBER, 0));
	};
};

class StringData : public vsData {
protected:
	std::string value;
public:
	StringData() :vsData(DataType::STRING), value("") {}

	StringData(const vsData& d) :vsData(d.getType()), value(d.toString()) { }

	StringData(const std::string data):vsData(DataType::STRING), value(data) { }

	// 比较的方法: 将d转换为string进行比较
	virtual bool operator==(const vsData& d) {
		bool ret = false;
		switch (d.getType())
		{
		case DataType::STRING:
			ret = value == d.toString();
			break;
		case DataType::NON:
		case DataType::NUMBER:
			ret = value == d.toString();
			break;
		case DataType::OPERA_ADDR:
			ret = value == d.toString();
			break;
		case DataType::BLK_INDEX:
		case DataType::ID_INDEX:
		case DataType::PARA_INDEX:
			ret = value == d.toString();
			break;
		default:
			break;
		}
		return ret;
	}

	virtual bool operator < (const vsData& d) {
		bool ret = false;
		switch (d.getType())
		{
		case DataType::STRING:
			ret = value < d.toString();
			break;
		case DataType::NON:
		case DataType::NUMBER:
			ret = value < d.toString();
			break;
		case DataType::OPERA_ADDR:
			ret = value < d.toString();
			break;
		case DataType::BLK_INDEX:
		case DataType::ID_INDEX:
		case DataType::PARA_INDEX:
			ret = value < d.toString();
			break;
		default:
			break;
		}
		return ret;
	}

	virtual bool operator > (const vsData& d) {
		bool ret = false;
		switch (d.getType())
		{
		case DataType::STRING:
			ret = value > d.toString();
			break;
		case DataType::NON:
		case DataType::NUMBER:
			ret = value > d.toString();
			break;
		case DataType::OPERA_ADDR:
			ret = value > d.toString();
			break;
		case DataType::BLK_INDEX:
		case DataType::ID_INDEX:
		case DataType::PARA_INDEX:
			ret = value > d.toString();
			break;
		default:
			break;
		}
		return ret;
	}

	// 回显用函数
	virtual std::string toEchoString() const {
		return value;
	}

	// 返回转换的字符串(支持数字转换为字符串)
	virtual std::string toString() const {
		return value;
	}

	// 返回转换的bool型(支持数字, 字符串转换bool)
	virtual bool toBool() const {
		return value != "";
	}

	// 返回转换的数字(支持字符串转换为数字)
	virtual long long toNumber() const {
		std::stringstream ss;
		ss << value;
		long long n;
		ss >> n;
		return n;
	}

	// 返回地址(只能是地址类型)
	virtual unsigned int toAddr() const {
		assert(false); // 不能返回地址
		return toNumber();
	}

	// 返回索引(只能是索引类型)
	virtual size_t toIndex() const {
		assert(false); // 不能返回索引
		return toNumber();
	}

	// 返回索引
	virtual data_ptr in(size_t index) override {
		std::string sub = value.substr(index, 1);
		return data_ptr(new StringData(sub));
	}
};

namespace NULL_DATA {
	const data_ptr null_data = data_ptr(new NumData());
}