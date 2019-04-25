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
	virtual void for_each(std::function<bool(std::string, data_ptr)> callback) = 0;
};

// 线性容器
class vsLinearContainer : public vsContainer {
protected:
	std::vector<data_ptr> _vec;
	int _it;
public:
	vsLinearContainer();
	
	vsLinearContainer(const vsLinearContainer& container);
		
	// 返回容器大小
	virtual size_t size();

	// 是否为空
	virtual bool empty() override;

	// 下一个对象(如果没有返回false, 如果还有返回true)
	virtual bool next()override;

	// 当前对象
	virtual data_ptr cur_data()override;

	// 当前键
	virtual data_ptr cur_key() override;

	// 从头开始
	virtual void gotoBegin() override;

	// 头部键值
	virtual data_ptr back_key() override;

	// 尾部键值
	virtual data_ptr head_key() override;

	// 会重写原有的映射关系, 重写返回false, 自动扩容或插入原先没有的元素系返回true
	virtual bool assign(data_ptr index, data_ptr value) override;

	// 不会重写原有的映射关系, 重写更改容器内的指针指向元素的值, 找到元素返回true, 没找到什么都不做, 并返回false
	virtual bool cp(data_ptr index, data_ptr value) override;

	// 返回找到的对象指针, 没有返回nullptr
	virtual data_ptr find(data_ptr index) override;


	// 全部清空
	virtual void clearAll() override;

	// 删除一个映射关系(会用空对象替代)
	virtual void clear(data_ptr index);

	virtual void for_each(std::function<bool(std::string, data_ptr)> callback);
};

// 标识符容器
class vsIdentifierContainer : public vsContainer {
protected:
	std::map<std::string, data_ptr> _map;

	std::map<std::string, data_ptr>::iterator _it;

public:
	vsIdentifierContainer();

	vsIdentifierContainer(const vsIdentifierContainer& container);
	
	// 返回容器大小
	virtual size_t size() override;

	// 是否为空
	virtual bool empty() override;

	// 下一个对象(如果没有返回false, 如果还有返回true)
	virtual bool next() override;

	// 当前对象
	virtual data_ptr cur_data()override;

	// 当前键
	virtual data_ptr cur_key()override;

	// 从头开始
	virtual void gotoBegin() override;

	// 头部键值
	virtual data_ptr back_key() override;

	// 尾部键值
	virtual data_ptr head_key() override;

	// 会重写原有的映射关系, 重写返回false, 自动扩容或插入原先没有的元素系返回true
	virtual bool assign(data_ptr index, data_ptr value)override;

	// 不会重写原有的映射关系, 重写更改容器内的指针指向元素的值, 找到元素返回true, 没找到什么都不做, 并返回false
	virtual bool cp(data_ptr index, data_ptr value) override;
	
	// 返回找到的对象指针
	virtual data_ptr find(data_ptr index) override;

	// 全部清空
	virtual void clearAll()override;

	// 删除一个映射关系(会用空对象替代)
	virtual void clear(data_ptr index)override;

	// for_each
	virtual void for_each(std::function<bool(std::string, data_ptr)> callback)override;
};
