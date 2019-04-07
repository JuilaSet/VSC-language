#pragma once

// 统一处理接口
class ObjectCreator {
protected:

public:
	ObjectCreator() = default;

};

//			//
//	对象	//
//			//

class _vsObject : public Data
{
	friend class ObjectCreator;
protected:
	size_t _ob_refcnt; // 引用计数
	_vsObject();
public:
	~_vsObject();
};
using vsObj_ptr = std::shared_ptr<_vsObject>;

//			//
//	数组	//
//			//

using vsVec_t = std::vector<size_t>;
class _vsVector : public _vsObject
{
	friend class ObjectCreator;
protected:
	vsVec_t _vec;
	_vsVector(vsVec_t vec) : _vec(vec) {}
public:

};