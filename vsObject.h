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
		assert(d->getType() >= DataType::OBJECT);
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
	std::shared_ptr<vsLinearContainer> _vec;

public:
	static vsVector* cast_vsVector_ptr(std::shared_ptr<vsData> d) {
		assert(d->getType() == DataType::OBJECT_VECTOR);
		vsVector* vecData = reinterpret_cast<vsVector*>(&*d);
		return vecData;
	}

	vsVector();

	vsVector(const vsVector& vec_obj);

	virtual data_ptr cp(std::shared_ptr<vsData> d) override;

	// 比较的方法
	virtual bool eq(std::shared_ptr<vsData> d) override;

	virtual bool l(std::shared_ptr<vsData> d) override;

	virtual bool g(std::shared_ptr<vsData> d) override;

	// 运算
	virtual std::shared_ptr<vsData> add(std::shared_ptr<vsData> d);

	// 回显用函数{ 0:x; 1:y; 2:z; ... }
	virtual std::string toEchoString() const override;

	// 返回转换的字符串(支持数字转换为字符串)
	virtual std::string toString() const override;

	// 返回大小的数字(支持字符串转换为数字)
	virtual long long toNumber() const override;

	// 返回转换的bool型(支持数字, 字符串转换bool)
	virtual bool toBool() const override;

	// 返回地址(只能是地址类型)
	virtual unsigned int toAddr() const override;

	// 返回索引(只能是索引类型)
	virtual size_t toIndex() const override;

public:
	// 查询成员(type == OBJECT) , 返回一个地址的引用 []][
	virtual data_ptr in(data_ptr index) override;
};

// 原始自定义对象(没有封装概念, 类似结构体)
class vsOriginObject : public vsObject 
{
protected:
	std::shared_ptr<vsIdentifierContainer> _public_map;

public:
	static vsOriginObject* cast_vsOriginObject_ptr(std::shared_ptr<vsData> d) {
		assert(d->getType() == DataType::OBJECT_ORIGIN);
		vsOriginObject* objData = reinterpret_cast<vsOriginObject*>(&*d);
		return objData;
	}

	vsOriginObject();

	vsOriginObject(const vsOriginObject& obj);

	// 潜复制对象
	virtual data_ptr cp(std::shared_ptr<vsData> d) override;

	// 比较的方法
	virtual bool eq(std::shared_ptr<vsData> d) override;

	virtual bool l(std::shared_ptr<vsData> d) override;

	virtual bool g(std::shared_ptr<vsData> d) override;

	// 运算
	virtual std::shared_ptr<vsData> add(std::shared_ptr<vsData> d) override;

	// 回显用函数{ 0:x; 1:y; 2:z; ... }
	virtual std::string toEchoString() const override;

	// 返回转换的字符串(支持数字转换为字符串)
	virtual std::string toString() const override;

	// 返回大小的数字(支持字符串转换为数字)
	virtual long long toNumber() const override;

	// 返回转换的bool型(支持数字, 字符串转换bool)
	virtual bool toBool() const override;

	// 返回地址(只能是地址类型)
	virtual unsigned int toAddr() const override;

	// 返回索引(只能是索引类型)
	virtual size_t toIndex() const override;

	// 查询成员, 索引的时候如果不存在就自动添加
	virtual data_ptr in(data_ptr index) override;
};