#pragma once

// 容器基类
class vsContainer
{
public:
	vsContainer();
	virtual ~vsContainer();

	// 返回容器大小
	virtual size_t size() = 0;

	// 是否为空
	virtual bool empty() = 0;

	// 下一个对象(如果没有返回false, 如果还有返回true)
	virtual bool next() = 0;

	// 当前对象
	virtual data_ptr cur_data() = 0;

	// 当前键
	virtual data_ptr cur_key() = 0;

	// 从头开始
	virtual void gotoBegin() = 0;

	// 头部键值
	virtual data_ptr back_key() = 0;

	// 尾部键值
	virtual data_ptr head_key() = 0;

	// 会重写原有的映射关系, 重写返回false, 自动扩容或插入原先没有的元素系返回true
	virtual bool assign(data_ptr, data_ptr) = 0;

	// 不会重写原有的映射关系, 重写更改容器内的指针指向元素的值, 找到元素返回true, 没找到什么都不做, 并返回false
	virtual bool cp(data_ptr, data_ptr) = 0;

	// 返回找到的对象指针, 没有返回nullptr
	virtual data_ptr find(data_ptr) = 0;
	
	// 全部清空
	virtual void clearAll() = 0;
	
	// 删除一个映射关系(会用空对象替代), std::out_of_range Inherited from std::exception
	virtual void clear(data_ptr) throw(std::out_of_range) = 0;

	// for_each(需要判断第二个参数是否为nullptr)
	virtual void for_each(std::function<void(std::string, data_ptr)> callback) = 0;
};

// 线性容器
class vsLinearContainer : public vsContainer {
protected:
	std::vector<data_ptr> _vec;
	int _it;
public:
	vsLinearContainer() {
		_it = 0;
	}
	
	// 返回容器大小
	virtual size_t size() {
		return _vec.size();
	}

	// 是否为空
	virtual bool empty() override {
		return _vec.empty();
	}

	// 下一个对象(如果没有返回false, 如果还有返回true)
	virtual bool next()override {
		++_it;
		return _it == _vec.size() ? false: true;
	}

	// 当前对象
	virtual data_ptr cur_data()override {
		return _vec[_it];
	}

	// 当前键
	virtual data_ptr cur_key() override {
		return data_ptr(new NumData(_it));
	}

	// 从头开始
	virtual void gotoBegin() override {
		_it = 0;
	}

	// 头部键值
	virtual data_ptr back_key() override {
		return data_ptr(new NumData(_vec.size()));
	}

	// 尾部键值
	virtual data_ptr head_key() {
		return data_ptr(new NumData(0));
	}

	// 会重写原有的映射关系, 重写返回false, 自动扩容或插入原先没有的元素系返回true
	virtual bool assign(data_ptr index, data_ptr value) override {
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
	virtual bool cp(data_ptr index, data_ptr value) override {
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
	virtual data_ptr find(data_ptr index) override {
		int i = index->toNumber();
		if (i < _vec.size())
			return _vec.at(i);
		else
			return nullptr;
	}

	// 全部清空
	virtual void clearAll() override {
		_vec.clear();
	}

	// 删除一个映射关系(会用空对象替代)
	virtual void clear(data_ptr index) throw(std::out_of_range) override {
		int i = index->toNumber();
		// throw
		_vec.at(i) = nullptr;
	}

	virtual void for_each(std::function<void(std::string, data_ptr)> callback) {
		auto begin = 0;
		auto end = _vec.size();
		for (auto it = begin; it != end; ++it) {
			if(_vec[it] != nullptr)
				callback(std::to_string(it), _vec[it]);
		}
	};
};

// 标识符容器
class vsIdentifierContainer : public vsContainer {
protected:
	std::map<std::string, data_ptr> _map;

	std::map<std::string, data_ptr>::iterator _it;

public:
	vsIdentifierContainer() {
		_it = _map.begin();
	}

	// 返回容器大小
	virtual size_t size() {
		return _map.size();
	}
	// 是否为空
	virtual bool empty() override {
		return _map.empty();
	}

	// 下一个对象(如果没有返回false, 如果还有返回true)
	virtual bool next() override {
		++_it;
		return _it == _map.end() ? false : true;
	}

	// 当前对象
	virtual data_ptr cur_data()override {
		return _it->second;
	}

	// 当前键
	virtual data_ptr cur_key()override {
		return data_ptr(new StringData(_it->first));
	}

	// 从头开始
	virtual void gotoBegin() override {
		_it = _map.begin();
	}

	// 头部键值
	virtual data_ptr back_key() override {
		return data_ptr(new StringData(_map.rbegin()->first));
	}

	// 尾部键值
	virtual data_ptr head_key() {
		return data_ptr(new StringData(_map.begin()->first));
	}

	// 会重写原有的映射关系, 重写返回false, 自动扩容或插入原先没有的元素系返回true
	virtual bool assign(data_ptr index, data_ptr value)override {
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
	virtual bool cp(data_ptr index, data_ptr value) override {
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
	virtual data_ptr find(data_ptr index) override {
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
	virtual void clearAll() {
		_map.clear();
	}

	// 删除一个映射关系(会用空对象替代)
	virtual void clear(data_ptr index) throw(std::out_of_range) override {
		std::string i = index->toString();
		// throw
		_map.erase(i);
	}

	// for_each
	virtual void for_each(std::function<void(std::string, data_ptr)> callback) {
		auto begin = _map.begin();
		auto end = _map.end();
		for (auto it = begin; it != end; ++it) {
			if (it->second != nullptr)
				callback(it->first, it->second);
		}
	};
};
