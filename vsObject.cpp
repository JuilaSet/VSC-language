#include "pch.h"
#include "vsObject.h"

vsVector::vsVector() : _vec(new vsLinearContainer), vsObject(DataType::OBJECT_VECTOR) { }

vsVector::vsVector(const vsVector& vec_obj) : _vec(new vsLinearContainer(*vec_obj._vec)), vsObject(DataType::OBJECT_VECTOR) { }

data_ptr vsVector::cp(std::shared_ptr<vsData> d) {
	assert(d->getType() == DataType::OBJECT_VECTOR);
	vsVector* vecData = cast_vsVector_ptr(d);
	*_vec = *vecData->_vec;
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
	auto& an_vec = vecData->_vec;
	// 新建对象
	auto ret_data = data_ptr(new vsVector(*this));
	vsVector* ret_vecData = cast_vsVector_ptr(ret_data);
	auto& ret_vec = ret_vecData->_vec;
	auto size = _vec->size();
	// 添加到元素尾部
	if (!an_vec->empty()) {
		an_vec->gotoBegin();
		do {
			data_ptr k = an_vec->cur_key();
			data_ptr d = an_vec->cur_data();
			if (d == nullptr)d = NULL_DATA::null_data;
			ret_vec->assign(k->add(data_ptr(new NumData(size))), d);
		} while (an_vec->next());
	}
	an_vec->gotoBegin();
#if CHECK_Object
	std::cout << __LINE__ << "\tRET_Object add result: " << ret_data->toEchoString() << std::endl;
#endif
	return ret_data;
}

// 回显用函数{ 0:x; 1:y; 2:z; ... }
std::string vsVector::toEchoString() const {
	std::string str = "{";
	if (!_vec->empty()) {
		_vec->for_each([&str](auto k, auto v) {
			if (v->getType() >= DataType::OBJECT) // 忽略子对象内容
				str += " " + k + ": { " + "..." + " };";
			else
				str += " " + k + ":" + v->toEchoString() + ";";
			return true;
		});
	}
	str += " }";
	return str;
}

// 返回转换的字符串(支持数字转换为字符串)
std::string vsVector::toString() const {
	std::string str = "{";
	if (!_vec->empty()) {
		_vec->for_each([&str](auto k, auto v) {
			if (v->getType() >= DataType::OBJECT) // 忽略子对象内容
				str += " " + k + ": { " + "..." + " };";
			else
				str += " " + k + ":" + v->toEchoString() + ";";
			if (str.size() > STRING_MAX_SZIE) {
				str += "...";
				return false;
			}
			return true;
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

//
// 原始自定义对象
//

vsOriginObject::vsOriginObject() 
	: _public_map(new vsIdentifierContainer), vsObject(DataType::OBJECT_ORIGIN) { }

vsOriginObject::vsOriginObject(const vsOriginObject& obj) 
	: _public_map(new vsIdentifierContainer(*obj._public_map)), vsObject(DataType::OBJECT_ORIGIN) { }

// 潜复制对象
data_ptr vsOriginObject::cp(std::shared_ptr<vsData> d) {
	assert(d->getType() == DataType::OBJECT_ORIGIN);
	vsOriginObject* objData = cast_vsOriginObject_ptr(d);
	*_public_map = *objData->_public_map;
	return data_ptr(new vsOriginObject(*this));
}

// 比较的方法
bool vsOriginObject::eq(std::shared_ptr<vsData> d) {
	assert(d->getType() == DataType::OBJECT_ORIGIN);
	vsOriginObject* objData = cast_vsOriginObject_ptr(d);
	return _public_map == objData->_public_map;
}

bool vsOriginObject::l(std::shared_ptr<vsData> d) {
	assert(d->getType() == DataType::OBJECT_ORIGIN);
	vsOriginObject* objData = cast_vsOriginObject_ptr(d);
	return _public_map < objData->_public_map;
}

bool vsOriginObject::g(std::shared_ptr<vsData> d) {
	assert(d->getType() == DataType::OBJECT_ORIGIN);
	vsOriginObject* objData = cast_vsOriginObject_ptr(d);
	return _public_map < objData->_public_map;
}

// 合并元素(会覆盖已有元素)
std::shared_ptr<vsData> vsOriginObject::add(std::shared_ptr<vsData> d) {
	assert(d->getType() == DataType::OBJECT_ORIGIN);
	vsOriginObject* objData = cast_vsOriginObject_ptr(d);
	auto& an_map = objData->_public_map;

	// 新建对象(复制自己)
	auto ret_obj = data_ptr(new vsOriginObject(*this));
	vsOriginObject* retData = cast_vsOriginObject_ptr(ret_obj);
	auto& ret_map = retData->_public_map;

	// 添加到元素尾部
	if (!an_map->empty()){
		an_map->gotoBegin();
		do {
			data_ptr k = an_map->cur_key();
			data_ptr d = an_map->cur_data();
			if (d == nullptr) d = NULL_DATA::null_data;
			ret_map->assign(k, d);
		} while (an_map->next());
	}
	an_map->gotoBegin();
	return ret_obj;
}

// 回显用函数{ 0:x; 1:y; 2:z; ... }
std::string vsOriginObject::toEchoString() const {
	std::string str = "{";
	if (!_public_map->empty()) {
		_public_map->for_each([&str, this](auto k, auto v) {
			if (v->getType() >= DataType::OBJECT) // 忽略子对象内容
				str += " " + k + ": { " + "..." + " };";
			else
				str += " " + k + ":" + v->toEchoString() + ";";
			return true;
		});
	}
	str += " }";
	return str;
}

// 返回转换的字符串(支持数字转换为字符串)
std::string vsOriginObject::toString() const {
	std::string str = "{";
	if (!_public_map->empty()) {
		_public_map->for_each([&str](auto k, auto v) {
			if (v->getType() >= DataType::OBJECT)	// 忽略子对象内容
				str += " " + k + ": { " + "..." + " };";
			else
				str += " " + k + ":" + v->toEchoString() + ";";
			if (str.size() > STRING_MAX_SZIE) {
				str += "...";
				return false;
			}
			return true;
		});
	}
	str += " }";
	return str;
}

// 返回大小的数字(支持字符串转换为数字)
long long vsOriginObject::toNumber() const {
	return _public_map->size();
}

// 返回转换的bool型(支持数字, 字符串转换bool)
bool vsOriginObject::toBool() const {
	return _public_map->empty();
}

// 返回地址(只能是地址类型)
unsigned int vsOriginObject::toAddr() const {
	assert(false);
	return 0;
}

// 返回索引(只能是索引类型)
size_t vsOriginObject::toIndex() const {
	assert(false);
	return 0;
}

// 查询成员, 返回查询的位置, 如果当前没有, 就从原型中继续查找
data_ptr vsOriginObject::in(data_ptr index) {
	auto delegation = new ContainerLocationData(_public_map, index);
	data_ptr data(delegation);
	return data;
}
