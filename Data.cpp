#include "pch.h"
#include "Data.h"

// 执行
int FunctionData::eval(vsEvaluator& evalor, int argc) {
	// 跳转block
	auto block_id = toIndex();
	auto vm = evalor.getVM();
	auto block_ptr = vm->get_block_ref(block_id);

#if	CHECK_Compiler_Field_NEW_BLK
	std::cerr << __LINE__ << " BLOCK_Field: " << (block_ptr->strong_hold() ? "T" : "F") << std::endl;
#endif

	evalor.load_block(block_ptr, argc);
	return 0;
}

// 构造函数
ContainerLocationData::ContainerLocationData(container_ptr container_p, data_ptr location) 
	: p_container(container_p), _location(location) {}

// 更改对容器该位置的索引
bool ContainerLocationData::container_assign(data_ptr value) {
	return p_container->assign(_location, value);
}

// 复制到容器的该位置
bool ContainerLocationData::container_cp(data_ptr value) {
	return p_container->cp(_location, value);
}

// 返回找到的对象指针, 没有返回空对象
data_ptr ContainerLocationData::container_find() {
	auto data = p_container->find(_location);
	return data == nullptr ? NULL_DATA::null_data : data;
}

std::shared_ptr<vsData> ContainerLocationData::cp(std::shared_ptr<vsData> d) {
	auto f = p_container->find(_location);
	if (f != nullptr)
		return f->cp(d);
	return NULL_DATA::null_data;
}

// 比较的方法
bool ContainerLocationData::eq(std::shared_ptr<vsData> d) {
	auto f = p_container->find(_location);
	if (f == nullptr) f = NULL_DATA::null_data;
	return f->eq(d);
}

bool ContainerLocationData::l(std::shared_ptr<vsData> d) {
	auto f = p_container->find(_location);
	if (f == nullptr) f = NULL_DATA::null_data;
	return f->l(d);
}

bool ContainerLocationData::g(std::shared_ptr<vsData> d) {
	auto f = p_container->find(_location);
	if (f == nullptr) f = NULL_DATA::null_data;
	return f->g(d);
}

// 运算
data_ptr ContainerLocationData::add(std::shared_ptr<vsData> d) {
	auto f = p_container->find(_location);
	if (f == nullptr) f = NULL_DATA::null_data;
	return f->add(d);
}

// 回显用函数
std::string ContainerLocationData::toEchoString() const {
	auto f = p_container->find(_location);
	if (f != nullptr)
		return f->toEchoString();
	return UNDEFINED_ECHO;
}

// 返回转换的字符串(支持数字转换为字符串)
std::string ContainerLocationData::toString() const {
	auto f = p_container->find(_location);
	if (f != nullptr)
		return f->toEchoString();
	return UNDEFINED_ECHO;
}

// 返回转换的数字(支持字符串转换为数字)
long long ContainerLocationData::toNumber() const {
	auto f = p_container->find(_location);
	if (f != nullptr)
		return f->toNumber();
	return 0;
}

// 返回转换的bool型(支持数字, 字符串转换bool)
bool ContainerLocationData::toBool() const {
	auto f = p_container->find(_location);
	if (f != nullptr)
		return f->toBool();
	return 0;
}

// 返回地址(只能是地址类型)
unsigned int ContainerLocationData::toAddr() const {
	auto f = p_container->find(_location);
	if (f != nullptr)
		return f->toAddr();
	return 0;
}

// 返回索引(只能是索引类型)
size_t ContainerLocationData::toIndex() const {
	auto f = p_container->find(_location);
	if (f != nullptr)
		return f->toIndex();
	return 0;
}