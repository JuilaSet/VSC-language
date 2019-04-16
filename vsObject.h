#pragma once


///
/// 异常
///

class index_out_of_range_exception : public std::exception
{
private:
	std::string _msg;
public:

	index_out_of_range_exception(const std::string msg) : _msg(msg) {}
	const char * what() const throw ()
	{
		return _msg.c_str();
	}
};

class vsObject : public vsData
{
public:
	static vsObject* cast_vsObject_ptr(std::shared_ptr<vsData> d) {
		vsObject* vsobj_data = reinterpret_cast<vsObject*>(&*d);
		return vsobj_data;
	}

	vsObject(DataType type = DataType::OBJECT) : vsData(type){ }

	// 查询成员, 索引的时候如果不存在就自动添加
	virtual data_ptr in(data_ptr index) = 0;
};

class vsVector : public vsObject
{
protected:
	container_ptr _vec;

public:
	static vsVector* cast_vsVector_ptr(std::shared_ptr<vsData> d) {
		vsVector* vecData = reinterpret_cast<vsVector*>(&*d);
		return vecData;
	}

	vsVector();

	vsVector(const vsVector& vec_obj);

	virtual data_ptr cp(std::shared_ptr<vsData> d);

	// 比较的方法
	virtual bool eq(std::shared_ptr<vsData> d);

	virtual bool l(std::shared_ptr<vsData> d);

	virtual bool g(std::shared_ptr<vsData> d);

	// 运算
	virtual std::shared_ptr<vsData> add(std::shared_ptr<vsData> d);

	// 回显用函数{ 0:x; 1:y; 2:z; ... }
	virtual std::string toEchoString() const;

	// 返回转换的字符串(支持数字转换为字符串)
	virtual std::string toString() const;

	// 返回大小的数字(支持字符串转换为数字)
	virtual long long toNumber() const;

	// 返回转换的bool型(支持数字, 字符串转换bool)
	virtual bool toBool() const;

	// 返回地址(只能是地址类型)
	virtual unsigned int toAddr() const;

	// 返回索引(只能是索引类型)
	virtual size_t toIndex() const;

public:
	// 查询成员(type == OBJECT) , 返回一个地址的引用 []][
	virtual data_ptr in(data_ptr index) override;
};
