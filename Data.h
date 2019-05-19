#pragma once

enum class DataType :int {
	// 空, enum默认构造为NON
	NON = 0,
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
	PARA_INDEX,
	// 函数类型
	FUNCTION,
	// 引用类型
	DELEGATION,
	// 用户自定义对象类型(>= object都是对象, 都有in方法)
	OBJECT,
	// vector对象
	OBJECT_VECTOR,
	// 原始自定义对象
	OBJECT_ORIGIN
};

// 数据对象基类
class vsData {
protected:
	DataType _type;
public:
	vsData(DataType type): _type(type) {}

	virtual std::shared_ptr<vsData> cp(std::shared_ptr<vsData> d) = 0;

	// 比较的方法
	virtual bool eq(std::shared_ptr<vsData> d) = 0;

	virtual bool l(std::shared_ptr<vsData> d) = 0;

	virtual bool g(std::shared_ptr<vsData> d) = 0;

	// 运算
	virtual std::shared_ptr<vsData> add(std::shared_ptr<vsData> d) = 0;

	// 回显用函数
	virtual std::string toEchoString() const = 0;

	// 返回转换的字符串(支持数字转换为字符串)
	virtual std::string toString() const = 0;

	// 返回转换的数字(支持字符串转换为数字)
	virtual long long toNumber() const = 0;

	// 返回转换的bool型(支持数字, 字符串转换bool)
	virtual bool toBool() const = 0; 

	// 返回地址(只能是地址类型)
	virtual unsigned int toAddr() const = 0; 

	// 返回索引(只能是索引类型)
	virtual size_t toIndex() const = 0; 

	// 返回类型
	virtual DataType getType() const {
		return _type;
	}

	// 返回类型名称
	virtual std::string getTypeName() const  {
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
		case DataType::FUNCTION:
			return "FUNCTION";
		case DataType::DELEGATION:
			return "DELEGATION";
		case DataType::OBJECT:
			return "OBJECT";
		case DataType::OBJECT_VECTOR:
			return "OBJECT_VECTOR";
		case DataType::OBJECT_ORIGIN:
			return "OBJECT_ORIGIN";
		default:
			assert(false); // 说明存在还没有注册的名称
			return "ERROR";
		}
	}

};

// 可执行对象接口
class IEvaluable: public vsData {
protected:
	RunTimeStackFrame_ptr runtime_context_ptr;	// 上下文环境(运行时栈帧)
public:
	static IEvaluable* cast_evaluable_ptr(std::shared_ptr<vsData> d) {
		assert(d->getType() == DataType::FUNCTION);
		return reinterpret_cast<IEvaluable*>(&*d);
	}

	IEvaluable() : vsData(DataType::FUNCTION) {}

	// 执行(计算对象, 函数参数个数)
	virtual int eval(vsEvaluator& evalor, int argc) = 0;

	// 设置当前上下文
	void set_runtime_ctx_ptr(RunTimeStackFrame_ptr ptr) {
		runtime_context_ptr = ptr;
	}

	// 获取外部运行上下文
	RunTimeStackFrame_ptr get_runtime_ctx_ptr() {
		return runtime_context_ptr;
	}
};

class NumData : public vsData {
protected:
	long long value;

public:
	NumData():vsData(DataType::NON), value(0) { }

	NumData(size_t data) : vsData(DataType::NUMBER), value(data) { }

	NumData(const vsData& d) :vsData(d.getType()), value(d.toNumber()) { }

	NumData(const NumData& d) :vsData(d.getType()), value(d.value) { }

	virtual data_ptr cp(data_ptr d) override {
		_type = DataType::NUMBER;
		value = d->toNumber();
		return data_ptr(new NumData(value));
	}

	// 比较
	virtual bool eq(data_ptr d) override {
		return value == d->toNumber();
	}

	virtual bool l(data_ptr d) override {
		return value < d->toNumber();
	}

	virtual bool g(data_ptr d) override {
		return value > d->toNumber();
	}

	// 运算
	virtual data_ptr add(data_ptr d) override {
		int v = value + d->toNumber();
		return data_ptr(new NumData(v)); // 返回NUMBER类型
	};

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

	// 返回数字(不论什么类型都可以)
	virtual long long toNumber() const override {
		return value;
	}

	// 返回转换的bool型(支持数字, 字符串转换bool)
	virtual bool toBool() const override {
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
};

class AddrData : public vsData {
protected:
	size_t value;

public:
	AddrData() :vsData(DataType::NON), value(0) { }

	AddrData(size_t data) : vsData(DataType::OPERA_ADDR), value(data) { }

	AddrData(const vsData& d) :vsData(d.getType()), value(d.toNumber()) { }

	AddrData(const AddrData& d) :vsData(d.getType()), value(d.value) { }

	virtual data_ptr cp(data_ptr d) override {
		_type = d->getType();
		assert(_type == DataType::OPERA_ADDR);
		value = d->toAddr();
		return data_ptr(new AddrData(value));
	}

	// 比较
	virtual bool eq(data_ptr d) override {
		return value == d->toNumber();
	}

	virtual bool l(data_ptr d) override {
		return value > d->toNumber();
	}

	virtual bool g(data_ptr d) override {
		return value == d->toNumber();
	}

	// 运算
	virtual data_ptr add(data_ptr d) override {
		int v = value + d->toNumber();
		return data_ptr(new NumData(v)); // 返回NUMBER类型
	};

	// 回显用函数
	virtual std::string toEchoString() const override {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	// 返回转换的字符串(支持数字转换为字符串)
	virtual std::string toString() const override {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	// 返回数字(不论什么类型都可以)
	virtual long long toNumber() const override {
		return value;
	}

	// 返回转换的bool型(支持数字, 字符串转换bool)
	virtual bool toBool() const override {
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
};

class IndexData : public vsData {
protected:
	std::string index;
public:
	IndexData(DataType type, std::string index) :vsData(type), index(index) { }

	IndexData(const vsData& d) :vsData(d.getType()), index(d.toString()) { }

	virtual data_ptr cp(data_ptr d) override {
		_type = d->getType();
		index = d->toString();
		return data_ptr(new IndexData(_type, index));
	}

	// 比较的方法: 将d转换为string进行比较
	virtual bool eq(data_ptr d) override {
		return index == d->toString();
	}

	virtual bool l(data_ptr d) override {
		return index < d->toString();
	}

	virtual bool g(data_ptr d) override {
		return index > d->toString();
	}

	// 运算
	virtual data_ptr add(data_ptr d) override {
		std::string v = index + d->toString();
		return data_ptr(new IndexData(_type, v));
	};

	// 回显用函数
	virtual std::string toEchoString() const {
		return index;
	}

	// 返回转换的字符串(支持数字转换为字符串)
	virtual std::string toString() const {
		return index;
	}

	// 返回转换的bool型(支持数字, 字符串转换bool)
	virtual bool toBool() const {
		return index != "";
	}

	// 返回转换的数字(支持字符串转换为数字)
	virtual long long toNumber() const {
		std::stringstream ss;
		ss << index;
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
		return toNumber();
	}
};

class StringData : public vsData {
protected:
	std::string value;
public:
	StringData() :vsData(DataType::STRING), value("") {}

	StringData(const vsData& d) :vsData(d.getType()), value(d.toString()) { }

	StringData(const std::string data):vsData(DataType::STRING), value(data) { }

	virtual data_ptr cp(data_ptr d) override {
		_type = DataType::STRING;
		value = d->toString();
		return data_ptr(new StringData(value));
	}

	// 比较的方法: 将d转换为string进行比较
	virtual bool eq(data_ptr d) override  {
		return value == d->toString();
	}

	virtual bool l(data_ptr d) override {
		return value < d->toString();
	}

	virtual bool g(data_ptr d) override {
		return value > d->toString();
	}

	// 运算
	virtual data_ptr add(data_ptr d) override {
		std::string v = value + d->toString();
		return data_ptr(new StringData(v)); // 返回NUMBER类型
	};

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
};

class FunctionData: public IEvaluable {
protected:
	size_t block_id;							// block代码块地址
public:

	FunctionData(size_t block_id) : block_id(block_id) {}

	virtual data_ptr cp(std::shared_ptr<vsData> d) override {
		assert(d->getType() == DataType::FUNCTION);
		block_id = d->toNumber();
		return data_ptr(new FunctionData(block_id));
	}

	// 比较的方法

	// 等于(指向同一个代码块)
	virtual bool eq(std::shared_ptr<vsData> d) override {
		return (d->getType() == _type) && (d->toIndex() == block_id);
	}

	virtual bool l(std::shared_ptr<vsData> d) override {
		return (d->getType() == _type) && (block_id < d->toIndex());
	}

	virtual bool g(std::shared_ptr<vsData> d) override {
		return (d->getType() == _type) && (block_id > d->toIndex());
	}

	// 运算
	virtual std::shared_ptr<vsData> add(std::shared_ptr<vsData> d) override {
		return data_ptr(new NumData(block_id + d->toNumber()));
	}

	// 回显用函数
	virtual std::string toEchoString() const override {
		std::stringstream ss;
		ss << "Function  " << "id=";
		ss << block_id;
		return ss.str();
	}

	// 返回转换的字符串(支持数字转换为字符串)
	virtual std::string toString() const override {
		assert(_type != DataType::OPERA_ADDR);
		assert(_type != DataType::ID_INDEX);
		assert(_type != DataType::BLK_INDEX);
		assert(_type != DataType::PARA_INDEX);
		std::stringstream ss;
		ss << block_id;
		return ss.str();
	}

	// 返回转换的数字(支持字符串转换为数字)
	virtual long long toNumber() const override {
		return block_id;
	}

	// 返回转换的bool型(支持数字, 字符串转换bool)
	virtual bool toBool() const override {
		return block_id != -1;
	}

	// 返回地址(只能是地址类型)
	virtual unsigned int toAddr() const override {
		assert(false);
		return block_id;
	}

	// 返回索引(只能是索引类型)
	virtual size_t toIndex() const override  {
		return block_id;
	}

public:
	// 执行
	virtual int eval(vsEvaluator& evalor, int argc) override;
};

class IDelegation : public vsData {
public:
	static IDelegation* cast_delegation_ptr(std::shared_ptr<vsData> d) {
		assert(d->getType() == DataType::DELEGATION);
		IDelegation* delegation_data = reinterpret_cast<IDelegation*>(&*d);
		return delegation_data;
	}

	IDelegation() : vsData(DataType::DELEGATION) {}

	// 会重写原有的映射关系, 重写返回false, 自动扩容或插入原先没有的元素系返回true
	virtual bool container_assign(data_ptr) = 0;

	// 不会重写原有的映射关系, 重写更改容器内的指针指向元素的值, 找到元素返回true, 没找到什么都不做, 并返回false
	virtual bool container_cp(data_ptr) = 0;

	// 返回找到的对象指针, 没有返回空对象
	virtual data_ptr container_find() = 0;

	// 删除该位置的指针
	virtual void container_del() = 0;
};

// 调用in指令时生成的对象, 用于索引容器位置
class ContainerLocationData : public IDelegation {
protected:
	container_ptr p_container;  // 容器指针
	data_ptr _location;			// 索引容器的位置
public:
	ContainerLocationData(container_ptr container_p, data_ptr location);

	// 更改对容器该位置的索引
	virtual bool container_assign(data_ptr value) override;

	// 复制到容器的该位置
	virtual bool container_cp(data_ptr value) override;

	virtual std::shared_ptr<vsData> cp(std::shared_ptr<vsData> d) override;

	// 返回找到的对象指针, 没有返回nullptr
	virtual data_ptr container_find() override;

	// 比较的方法
	virtual bool eq(std::shared_ptr<vsData> d) override;

	virtual bool l(std::shared_ptr<vsData> d) override;

	virtual bool g(std::shared_ptr<vsData> d) override;

	// 运算
	virtual data_ptr add(std::shared_ptr<vsData> d) override;

	// 回显用函数
	virtual std::string toEchoString() const override;

	// 返回转换的字符串(支持数字转换为字符串)
	virtual std::string toString() const override;

	// 返回转换的数字(支持字符串转换为数字)
	virtual long long toNumber() const override;

	// 返回转换的bool型(支持数字, 字符串转换bool)
	virtual bool toBool() const override;

	// 返回地址(只能是地址类型)
	virtual unsigned int toAddr() const override;

	// 返回索引(只能是索引类型)
	virtual size_t toIndex() const override;

	// 删除该位置的指针
	virtual void container_del() override;
};

namespace NULL_DATA {
	const data_ptr null_data = data_ptr(new NumData());
}