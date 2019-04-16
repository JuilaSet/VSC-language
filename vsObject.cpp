#include "pch.h"
#include "vsObject.h"

vsVector::vsVector() : _vec(new vsLinearContainer), vsObject(DataType::OBJECT_VECTOR) { }

vsVector::vsVector(const vsVector& vec_obj) : vsObject(DataType::OBJECT_VECTOR) {
	_vec = vec_obj._vec;
}

data_ptr vsVector::cp(std::shared_ptr<vsData> d) {
	assert(d->getType() == DataType::OBJECT_VECTOR);
	vsVector* vecData = cast_vsVector_ptr(d);
	_vec = vecData->_vec;
	return data_ptr(new vsVector(*this));
}

// 比较的方法
bool vsVector::eq(std::shared_ptr<vsData> d) {
	assert(d->getType() == DataType::OBJECT_VECTOR);
	vsVector* vecData = cast_vsVector_ptr(d);
	return _vec == vecData->_vec;
}

bool vsVector::l(std::shared_ptr<vsData> d) {
	assert(d->getType() == DataType::OBJECT_VECTOR);
	vsVector* vecData = cast_vsVector_ptr(d);
	return _vec < vecData->_vec;
}

bool vsVector::g(std::shared_ptr<vsData> d) {
	assert(d->getType() == DataType::OBJECT_VECTOR);
	vsVector* vecData = cast_vsVector_ptr(d);
	return _vec > vecData->_vec;
}

// 运算
std::shared_ptr<vsData> vsVector::add(std::shared_ptr<vsData> d) {
	assert(d->getType() == DataType::OBJECT_VECTOR);
	vsVector* vecData = cast_vsVector_ptr(d);
	auto an_vec = vecData->_vec;
	// 添加到元素尾部
	if (!an_vec->empty())
		do {
			data_ptr k = _vec->cur_key();
			auto d = _vec->cur_data();
			_vec->assign(k->add(_vec->back_key()), d);
		} while (_vec->next());
	
	return data_ptr(new vsVector(*this));
}

// 回显用函数{ 0:x; 1:y; 2:z; ... }
std::string vsVector::toEchoString() const {
	std::string str = "{ ";
	if (!_vec->empty()) {
		_vec->for_each([&str](auto k, auto v) {
			str += " " + k + ":" + v->toEchoString() + ";";
		});
	}
	str += " }";
	return str;
}

// 返回转换的字符串(支持数字转换为字符串)
std::string vsVector::toString() const {
	std::string str = "{ ";
	if (!_vec->empty()) {
		_vec->for_each([&str](auto k, auto v) {
			str += " " + k + ":" + v->toEchoString() + ";";
		});
	}
	str += " }";
	return str;
}

// 返回大小的数字(支持字符串转换为数字)
long long vsVector::toNumber() const {
	return _vec->size();
}

// 返回转换的bool型(支持数字, 字符串转换bool)
bool vsVector::toBool() const {
	return _vec->empty();
}

// 返回地址(只能是地址类型)
unsigned int vsVector::toAddr() const {
	assert(false);
	return 0;
}

// 返回索引(只能是索引类型)
size_t vsVector::toIndex() const {
	assert(false);
	return 0;
}

// 查询成员, 返回查询的位置
data_ptr vsVector::in(data_ptr index) {
	return data_ptr(new ContainerLocationData(_vec, index));
}