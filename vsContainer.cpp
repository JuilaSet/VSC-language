#include "pch.h"
#include "vsContainer.h"


vsContainer::vsContainer()
{
}

vsContainer::~vsContainer()
{
}

// LinearConatiner

vsLinearContainer::vsLinearContainer(): _it(0) { }

vsLinearContainer::vsLinearContainer(const vsLinearContainer& container) :_vec(container._vec), _it(0) { }

// 返回容器大小
size_t vsLinearContainer::size() {
	return _vec.size();
}

// 是否为空
bool vsLinearContainer::empty()  {
	return _vec.empty();
}

// 下一个对象(如果没有返回false, 如果还有返回true)
bool vsLinearContainer::next() {
	++_it;
	return _it == _vec.size() ? false : true;
}

// 当前对象
data_ptr vsLinearContainer::cur_data() {
	return _vec[_it];
}

// 当前键
data_ptr vsLinearContainer::cur_key()  {
	return data_ptr(new NumData(_it));
}

// 从头开始
void vsLinearContainer::gotoBegin()  {
	_it = 0;
}

// 尾部键值
data_ptr vsLinearContainer::back_key()  {
	return data_ptr(new NumData(_vec.size() - 1));
}

// 头部键值
data_ptr vsLinearContainer::head_key() {
	return data_ptr(new NumData(0));
}

// 会重写原有的映射关系, 重写返回false, 自动扩容或插入原先没有的元素系返回true
bool vsLinearContainer::assign(data_ptr index, data_ptr value)  {
	int i = index->toNumber();
	bool ret = true;
	if (i < _vec.size()) {
		// 当前位置非空, 更新了元素, 返回false
		if (_vec[i] != nullptr) {
			ret = false;
		}
		_vec[i] = value;
	}
	else {
		_vec.resize(i + 1, nullptr);
		_vec[i] = value;
	}
	return ret;
}

// 不会重写原有的映射关系, 重写更改容器内的指针指向元素的值, 找到元素返回true, 没找到什么都不做, 并返回false
bool vsLinearContainer::cp(data_ptr index, data_ptr value)  {
	int i = index->toNumber();
	bool ret = false;
	if (i < _vec.size()) {
		if (_vec[i] != nullptr) {
			ret = true;
			_vec[i]->cp(value);
		}
	}
	return ret;
}

// 返回找到的对象指针, 没有返回nullptr
data_ptr vsLinearContainer::find(data_ptr index)  {
	int i = index->toNumber();
	if (i < _vec.size())
		return _vec.at(i);
	else
		return nullptr;
}

// 全部清空
void vsLinearContainer::clearAll()  {
	_vec.clear();
}

// 删除一个映射关系(会用空对象替代)
void vsLinearContainer::clear(data_ptr index) throw(std::out_of_range)  {
	int i = index->toNumber();
	// throw
	_vec.at(i) = nullptr;
}

void vsLinearContainer::for_each(std::function<bool(std::string, data_ptr)> callback) {
	auto begin = 0;
	auto end = _vec.size();
	for (auto it = begin; it != end; ++it) {
		if (_vec[it] != nullptr)
			if (callback(std::to_string(it), _vec[it]))break;
	}
};

// 标识符容器

vsIdentifierContainer::vsIdentifierContainer() :_it(_map.begin()) { }

vsIdentifierContainer::vsIdentifierContainer(const vsIdentifierContainer& container) :_map(container._map), _it(_map.begin()) { }

// 返回容器大小
size_t vsIdentifierContainer::size() {
	return _map.size();
}
// 是否为空
bool vsIdentifierContainer::empty() {
	return _map.empty();
}

// 下一个对象(如果没有返回false, 如果还有返回true)
bool vsIdentifierContainer::next() {
	++_it;
	return _it == _map.end() ? false : true;
}

// 当前对象
data_ptr vsIdentifierContainer::cur_data() {
	return _it->second;
}

// 当前键
data_ptr vsIdentifierContainer::cur_key() {
	return data_ptr(new StringData(_it->first));
}

// 从头开始
void vsIdentifierContainer::gotoBegin() {
	_it = _map.begin();
}

// 头部键值
data_ptr vsIdentifierContainer::back_key() {
	return data_ptr(new StringData(_map.rbegin()->first));
}

// 尾部键值
data_ptr vsIdentifierContainer::head_key() {
	return data_ptr(new StringData(_map.begin()->first));
}

// 会重写原有的映射关系, 重写返回false, 自动扩容或插入原先没有的元素系返回true
bool vsIdentifierContainer::assign(data_ptr index, data_ptr value) {
	std::string i = index->toString();
	bool ret = true;
	auto it = _map.find(i);
	if (it != _map.end()) {
		ret = false;
	}
	// 更新映射关系
	_map.insert_or_assign(i, value);
	return ret;
}

// 不会重写原有的映射关系, 重写更改容器内的指针指向元素的值, 找到元素返回true, 没找到什么都不做, 并返回false
bool vsIdentifierContainer::cp(data_ptr index, data_ptr value) {
	std::string i = index->toString();
	bool ret = false;
	auto it = _map.find(i);
	if (it != _map.end()) {
		auto& old_v = it->second;
		if (old_v != nullptr) {
			ret = true;
			old_v->cp(value);
		}
	}
	return ret;
}

// 返回找到的对象指针
data_ptr vsIdentifierContainer::find(data_ptr index) {
	std::string i = index->toString();
	// throw
	auto it = _map.find(i);
	if (it != _map.end()) {
		return it->second;
	}
	else {
		return nullptr;
	}
}

// 全部清空
void vsIdentifierContainer::clearAll() {
	_map.clear();
}

// 删除一个映射关系(会用空对象替代)
void vsIdentifierContainer::clear(data_ptr index) throw(std::out_of_range) {
	std::string i = index->toString();
	// throw
	_map.erase(i);
}

// for_each (规则: 返回值决定是否停止)
void vsIdentifierContainer::for_each(std::function<bool(std::string, data_ptr)> callback) {
	auto begin = _map.begin();
	auto end = _map.end();
	for (auto it = begin; it != end; ++it) {
		if (it->second != nullptr)
			if(callback(it->first, it->second))break;
	}
};

// 字典容器

