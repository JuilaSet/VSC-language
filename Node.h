#pragma once

// 没有子结点的结点
class LeafNode
{
	friend bool operator<(const LeafNode& l1, const LeafNode& l2);
	friend bool operator==(const LeafNode& l1, const LeafNode& l2);

public:
	static std::shared_ptr<LeafNode> create(std::string name = "") {
		return std::shared_ptr<LeafNode>(new LeafNode(name));
	}

protected:
	// id, 表示在图中的ID
	vsTool::id_t _id;

	// 名称
	const std::string _name;

	// 数据表(属性 : 结果)
	std::map<std::string, data_ptr> _data_map;

protected:
	// constructor
	LeafNode(std::string name) : _id(-1), _name(name) { }
	LeafNode(vsTool::id_t id, std::string name) : _id(id), _name(name) { }

public:
#if CHECK_Compiler_Node
	void show() {
		// 遍历所有的数据
		for (auto &pair : _data_map) {
			std::cout << pair.first << " : " <<  pair.second->toEchoString() << std::endl;
		}
	}
#endif

	// 设置数据
	void pushData(const std::string& key, data_ptr data) {
		_data_map.insert_or_assign(key, data);
	}

	// 设置ID
	void setID(vsTool::id_t id) {
		_id = id;
	}

	// 获取ID
	const vsTool::id_t& getId() const {
		return _id;
	}

	// 获取名称
	const std::string& getName() const {
		return _name;
	}

	// 获取数据
	data_ptr getData(const std::string& key) const {
		auto fit = _data_map.find(key);
		if (fit != _data_map.end())
			return fit->second;
		else
			return NULL_DATA::null_data;
	}

	// 获取数据集合
	const decltype(_data_map)& dataSet() {
		return _data_map;
	}
};