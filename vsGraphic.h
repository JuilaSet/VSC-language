#pragma once

#define CHECK_Tool_graphic true

namespace vsTool {

	//
	// 前向声明
	//

	using id_t = unsigned int;

	template<class CONTAIN_TYPE>
	class _GNode;

	template<class CONTAIN_TYPE>
	class _Graphic;

	template<class CONTAIN_TYPE>
	class GraphicBuilder;

	template<class CONTAIN_TYPE>
	using node_ptr = std::shared_ptr<_GNode<CONTAIN_TYPE>>;

	template<class CONTAIN_TYPE>
	using graphic_ptr = std::shared_ptr<_Graphic<CONTAIN_TYPE>>;

	template<class CONTAIN_TYPE>
	using graphic_builder_ptr = std::shared_ptr<GraphicBuilder<CONTAIN_TYPE>>;

	template<class CONTAIN_TYPE>
	using node_vec = std::vector<node_ptr<CONTAIN_TYPE>>;

	template<class CONTAIN_TYPE>
	using node_map = std::map<id_t, node_ptr<CONTAIN_TYPE>>;

	//
	// class 
	//

	// 构建模式

	enum class BUILD_MODE
	{
		AUTO_MOVE = 0,	// 图建造者的指针会自动转到下一个创建的结点
		STATIC = 1		// 图建造者的指针不会动跳转
	};


	// 结点对象(规则: 存放的data必须是 storeable 的, 并且有默认构造函数, id为-1是空对象)

	template<class CONTAIN_TYPE>
	class _GNode
	{
		friend class _Graphic<CONTAIN_TYPE>;
		friend class GraphicBuilder<CONTAIN_TYPE>;
	protected:

		id_t id;							// 结点下标, 对应着数组中的下标
		CONTAIN_TYPE data;					// 保存的数据, data必须是 storeable 的, 并且有默认构造函数
		std::set<id_t> out_nodes_id;		// 所有出去的结点下标
		std::set<id_t> in_nodes_id;			// 所有进入的结点下标

		// 构造方法
		_GNode(id_t id) : id(id) {}

		// 构造方法
		_GNode(id_t id, CONTAIN_TYPE data) : id(id), data(data) {}

	public:

		// 结点的入度
		size_t get_in_degree() const {
			return in_nodes_id.size();
		}

		// 结点的出度
		size_t get_out_degree() const {
			return out_nodes_id.size();
		}

		// 结点的id
		id_t get_id() {
			return id;
		}

		// 设置结点数据
		void set_data(CONTAIN_TYPE data) {
			this->data = data;
		}

		// 结点的数据
		CONTAIN_TYPE get_data() const {
			return data;
		}

		// 是否是空结点
		bool is_empty() const {
			return id == -1;
		}

	};


	// 图: 

	template<class CONTAIN_TYPE>
	class _Graphic
	{
		friend class GraphicBuilder<CONTAIN_TYPE>;
	protected:

		static node_ptr<CONTAIN_TYPE> _Empty_node;	// 空对象

		const node_vec<CONTAIN_TYPE> nodes;			// 所有结点的集合, 可能有下标对应的值为空(nullptr)

		bool _is_loop;								// 图类型--是否有回路
		bool _is_directed;							// 图类型--是否是有向图

	protected:
		// 图是否包含结点
		bool _contain_node(id_t id) {
			return id < nodes.size() && nodes[id] != nullptr;
		}

		// 获取结点
		node_ptr<CONTAIN_TYPE> get_node(id_t id) {
			assert(_contain_node(id));
			return nodes[id];
		}

		// 计算_is_loop, 判断结点是否是环
		void _calc_is_loop() {
			_is_loop = false;
			// 存放所有的祖先结点
			std::set<id_t> set_index;
			// 通过dfs遍历时, 如果该结点有边指向师祖结点, 就是一个环
			this->for_each_dfs(
				// 取出带拓展的结点, 该结点下标放入标记数组, 放入栈中
				[&set_index](auto index) {
				set_index.insert(index);
				return true;
			},
				// 拓展的结点, 如果生成的结点在祖先结点中, 说明是一个环
				[&set_index, this](auto index) {
				if (set_index.find(index) != set_index.end()) {
					this->_is_loop = true;
					return false;
				}
				return true;
			});
		}

		// 计算 _is_directed, 判断图是否是有向图(即每一个结点都存在一个出与一个入相同的下标)
		void _calc_is_directed() {
			_is_directed = false;
			for (auto n_ptr : nodes) {
				for (auto in_index : n_ptr->in_nodes_id) {
					auto it = n_ptr->out_nodes_id.find(in_index);
					if (it == n_ptr->out_nodes_id.end()) {
						_is_directed = true;
					}
				}
			}
		}

		// 构造函数(保存结点的索引信息)
		_Graphic(node_vec<CONTAIN_TYPE> nodes) : nodes(nodes) {
			// 计算是否是环
			_calc_is_loop();
			// 判断是否是有向图
			_calc_is_directed();
		}

	public:

		// 图是否有回路
		bool isLoop() {
			return _is_loop;
		}

		// 图是否是有向的
		bool isDirected() {
			return _is_directed;
		}

		// 判断两个图是否同构 []][
		// bool isomorphism(const _Graphic<CONTAIN_TYPE>& _g);

		//
		// 图的遍历
		//

		// 无序遍历(规则: 传入函数为void (node_ptr<CONTAIN_TYPE>))
		void for_each_unordered(std::function<void(node_ptr<CONTAIN_TYPE>)> func) {
			for (auto n : nodes) {
				if (n != nullptr)
					func(n);
			}
		}

		// DFS遍历(规则: 函数返回false停止遍历, 返回true继续)
		void for_each_dfs(std::function<bool(id_t index)> func, std::function<bool(id_t index)> func_ext) {
			// 存放待拓展的结点
			std::vector<id_t> stk_index;
			// 取出首部结点下标, 放入stk
			stk_index.push_back(0);
			// 一一遍历, 直到stk为空为止
			while (!stk_index.empty()) {
				// 取出一个结点下标
				id_t index = stk_index.back();
				stk_index.pop_back();
				// 获取这个结点
				auto node = get_node(index);
				// 调用待扩展结点的回调函数
				if (!func(index)) goto End;
				// 获取它所有的后继结点下标
				for (id_t i : node->out_nodes_id) {
					// 调用回调函数
					if (!func_ext(i)) goto End;
					// 取出结点下标
					stk_index.push_back(i);
				}
			}
		End:;
		}

		//
		// 数据访问
		//

		// 返回数据: 如果不存在为nullptr
		node_ptr<CONTAIN_TYPE> get_node_ptr(id_t index) {
			if (index < nodes.size()) {
				auto& node = nodes[index];
				return node != nullptr ? node : _Empty_node;
			}
			else {
				return _Empty_node;
			}
		}

		// 获取指向index结点的所有结点
		node_vec<CONTAIN_TYPE> get_nodes_point(id_t index) {
			if (!_contain_node(index))return node_vec<CONTAIN_TYPE>();
			// 存在结点
			auto node_aim = nodes[index];
			node_vec<CONTAIN_TYPE> ret;
			for (auto id : node_aim->in_nodes_id) {
				ret.push_back(nodes[id]);
			}
			return ret;
		}

		// 获取被index结点指向的所有结点
		node_vec<CONTAIN_TYPE> get_nodes_pointed_by(id_t index) {
			if (!_contain_node(index))return node_vec<CONTAIN_TYPE>();
			// 存在结点
			auto node_aim = nodes[index];
			node_vec<CONTAIN_TYPE> ret;
			for (auto id : node_aim->out_nodes_id) {
				ret.push_back(nodes[id]);
			}
			return ret;
		}

#if CHECK_Tool_graphic
		// 回显
		void show() {
			// 输出图的类型
			std::cout
				<< "Graphic: "
				<< (_is_directed ? "是一个有向" : "是一个无向")
				<< (_is_loop ? "有环图" : "无环图")
				<< std::endl;
			// 进行无序遍历
			for (auto n : nodes) {
				if (n != nullptr) {
					std::cout << "NODE_ID: " << n->id << " in(" << n->get_in_degree() << ")" << " out(" << n->get_out_degree() << ")" << std::endl;
					for (auto& out_id : n->out_nodes_id) {
						std::cout << n->id << " -> " << out_id << std::ends;
					}
					for (auto& in_id : n->in_nodes_id) {
						std::cout << n->id << " <- " << in_id << std::ends;
					}
					std::cout << std::endl;
				}
			}
		}
#endif
	};


	// 初始化空对象
	template<class CONTAIN_TYPE>
	node_ptr<CONTAIN_TYPE> _Graphic<CONTAIN_TYPE>::_Empty_node(new _GNode<CONTAIN_TYPE>(-1));


	// 图建造者: 

	template<class CONTAIN_TYPE>
	class GraphicBuilder
	{
	protected:

		node_vec<CONTAIN_TYPE> nodes;			// 所有结点的集合
		id_t index;								// 建造图时所在结点的下标
		BUILD_MODE mode;						// 构建模式

	protected:

		// 模式行动: 每一次创建结点完毕后会调用
		void _mode_action(id_t next_id) {
			switch (mode) {
			case BUILD_MODE::AUTO_MOVE:
				index = next_id;
				break;
			case BUILD_MODE::STATIC:
				break;
			default:
				assert(false);
			}
		}

		// 图是否包含结点
		bool _contain_node(id_t id) {
			return id < nodes.size() && nodes[id] != nullptr;
		}

		// 创建结点
		node_ptr<CONTAIN_TYPE> _create_node(id_t id, CONTAIN_TYPE data) {
			_GNode<CONTAIN_TYPE>* node = new _GNode<CONTAIN_TYPE>(id, data);
			return node_ptr<CONTAIN_TYPE>(node);
		}

	public:

		// 构造函数
		GraphicBuilder(BUILD_MODE m = BUILD_MODE::AUTO_MOVE) :nodes(1), index(-1), mode(m) {}

		// 设置构造模式
		void setMode(BUILD_MODE m) {
			mode = m;
		}

		//
		// 构造图的方法: 执行完毕后, 指针指向下一个位置
		//

		// 头(规则: 头结点必须在最开始)
		GraphicBuilder<CONTAIN_TYPE>* head(CONTAIN_TYPE data) {
			assert(index == -1); // 头结点必须在最开始
			auto node = _create_node(0, data);
			nodes[0] = node;
			index = 0;
			return this;
		}

		// 添加一个结点, 当前结点指向这个结点, 单向(规则: 之前没有定义过这个结点)
		GraphicBuilder<CONTAIN_TYPE>* addNode(id_t id, CONTAIN_TYPE data) {
			// 之前没有定义过这个结点
			assert(!_contain_node(id));
			auto node = _create_node(id, data);

			// 设置原有的结点的邻接结点
			auto cur_node = nodes[index];
			cur_node->out_nodes_id.insert(id);
			node->in_nodes_id.insert(index);

			// 如果大小不够就分配大小
			if (id >= nodes.size()) {
				nodes.resize(id + 1, nullptr);
				nodes[id] = node;
			}
			else {
				nodes[id] = node;
			}

			// 根据对应的模式行动
			_mode_action(node->id);
			return this;
		}

		// 添加一个结点, 当前结点与这个结点相互指向, 双向(规则: 之前没有定义过这个结点)
		GraphicBuilder<CONTAIN_TYPE>* addNode_bi(id_t id, CONTAIN_TYPE data) {
			// 之前没有定义过这个结点
			assert(!_contain_node(id));
			auto node = _create_node(id, data);
			auto cur_node = nodes[index];

			// 建立双向连接
			node->out_nodes_id.insert(index);
			cur_node->in_nodes_id.insert(id);

			cur_node->out_nodes_id.insert(id);
			node->in_nodes_id.insert(index);

			// 如果大小不够就分配大小
			if (id >= nodes.size()) {
				nodes.resize(id + 1, nullptr);
				nodes[id] = node;
			}
			else {
				nodes[id] = node;
			}

			// 根据对应的模式行动
			_mode_action(node->id);
			return this;
		}

		// 转到到结点(规则: 转到的位置一定存在)
		GraphicBuilder<CONTAIN_TYPE>* gotoNode(id_t id) {
			assert(_contain_node(id));	// 转到的位置一定存在
			index = id;
			return this;
		}

		// 结点指向结点(规则: 指向的位置一定存在, 并且之前不存在指向这条边)
		GraphicBuilder<CONTAIN_TYPE>* linkToNode(id_t dest_id) {
			auto cur_node = nodes[index];
			// 指向的位置一定存在
			assert(_contain_node(dest_id));
			auto dest_node = nodes[dest_id];
			// 之前应该不存在指向这条边
			auto found = cur_node->out_nodes_id.find(dest_id);
			assert(found == cur_node->out_nodes_id.end());
			// 建立单向连接
			cur_node->out_nodes_id.insert(dest_id);
			dest_node->in_nodes_id.insert(index);
			// 根据对应的模式行动
			_mode_action(dest_id);
			return this;
		}

		// 结点与结点相互指向(规则: 指向的位置一定存在, 并且之前不存在这两条边)
		GraphicBuilder<CONTAIN_TYPE>* linkToNode_bi(id_t dest_id) {
			auto cur_node = nodes[index];

			assert(_contain_node(dest_id));	// 指向的位置一定存在
			auto dest_node = nodes[dest_id];

			// cur -> dest 边不存在
			auto found_cur_dest = cur_node->out_nodes_id.find(dest_id);
			assert(found_cur_dest == cur_node->out_nodes_id.end());
			// 建立单向连接
			cur_node->out_nodes_id.insert(dest_id);
			dest_node->in_nodes_id.insert(index);

			// dest -> cur 边不存在
			auto found_dest_cur = dest_node->out_nodes_id.find(index);
			assert(found_dest_cur == dest_node->out_nodes_id.end());
			// 建立单向连接
			dest_node->out_nodes_id.insert(index);
			cur_node->in_nodes_id.insert(dest_id);

			// 根据对应的模式行动
			_mode_action(dest_id);
			return this;
		}

		// 建造这个图
		graphic_ptr<CONTAIN_TYPE> getGraphic() {
			auto g = new _Graphic<CONTAIN_TYPE>(nodes);
			return graphic_ptr<CONTAIN_TYPE>(g);
		}

		// 初始化
		void init() {
			nodes.clear();
			index = -1;
		}
	};

}