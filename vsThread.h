#pragma once

namespace vsThread {

	template<class KEY_TYPE, class DATA_TYPE>
	class Session;

	template<class KEY_TYPE, class DATA_TYPE>
	using session_ptr = std::shared_ptr<Session<KEY_TYPE, DATA_TYPE>>;

	template<class KEY_TYPE, class DATA_TYPE>
	class TaskGraphic;

	template<class KEY_TYPE, class DATA_TYPE>
	using taskGraphic_ptr = std::shared_ptr<TaskGraphic<KEY_TYPE, DATA_TYPE>>;

	template<class ID_TYPE, class KEY_TYPE, class DATA_TYPE>
	class _Thread_Node;

	template<class ID_TYPE, class KEY_TYPE, class DATA_TYPE>
	using _thread_node_ptr = std::shared_ptr<_Thread_Node<ID_TYPE, KEY_TYPE, DATA_TYPE>>;

	// 返回信号键
	template<class KEY_TYPE, class DATA_TYPE>
	using node_mapping_func = std::function<KEY_TYPE(DATA_TYPE&, std::map<KEY_TYPE, DATA_TYPE>&, std::set<KEY_TYPE>&)>;

	// 信号(键值 + 数据)
	template<class ID_TYPE, class KEY_TYPE, class DATA_TYPE>
	class _Signal {
		// 只有线程结点才可以获取
		friend class _Thread_Node<ID_TYPE, KEY_TYPE, DATA_TYPE>;

		friend bool operator<(const _Signal<ID_TYPE, KEY_TYPE, DATA_TYPE>& s1, const _Signal<ID_TYPE, KEY_TYPE, DATA_TYPE>& s2) {
			return s1._key < s2._key;
		}

	protected:
		KEY_TYPE _key;
		DATA_TYPE _data;

	public:
		_Signal(KEY_TYPE key, DATA_TYPE data) : _key(key), _data(data) {	}
	};

	// 传递数据的位置信息
	template<class ID_TYPE, class KEY_TYPE, class DATA_TYPE>
	class _Node_Position {
		// 只有线程结点才可以获取
		friend class _Thread_Node<ID_TYPE, KEY_TYPE, DATA_TYPE>;

		friend bool operator<(const _Node_Position<ID_TYPE, KEY_TYPE, DATA_TYPE>& p1, const _Node_Position<ID_TYPE, KEY_TYPE, DATA_TYPE>& p2) {
			return p1._key < p2._key;
		}

	protected:
		KEY_TYPE _key;
		_thread_node_ptr<ID_TYPE, KEY_TYPE, DATA_TYPE> node_ptr;

	public:
		_Node_Position(KEY_TYPE key, _thread_node_ptr<ID_TYPE, KEY_TYPE, DATA_TYPE> ptr) :_key(key), node_ptr(ptr) {}

	};

	// 定时器
	class Timer {
	protected:
		std::mutex _mutex;
		std::condition_variable _expired_cv;

	public:

		// 同步执行一次
		template<typename callable, class... arguments>
		void SyncWait(int after, callable&& f, arguments&&... args) {
			std::function<typename std::result_of<callable(arguments...)>::type()> task
			(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));
			// 执行时就停止after秒
			std::this_thread::sleep_for(std::chrono::milliseconds(after));
			task();
		}

		// 异步执行一次
		template<typename callable, class... arguments>
		void AsyncWait(int after, callable&& f, arguments&&... args) {
			std::function<typename std::result_of<callable(arguments...)>::type()> task
			(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));
			// 执行时, 启动一个线程让其停止after秒之后执行
			std::thread([after, task]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(after));
				task();
			}).detach();
		}
	};

	// 任务类(规则: ID_TYPE必须是可赋值的, KEY_TYPE必须是可赋值 /\ 可比较的, DATA_TYPE必须有空对象 /\ 必须是可赋值的)
	template<class ID_TYPE, class KEY_TYPE, class DATA_TYPE>
	class _Thread_Node {
	protected:
		ID_TYPE _id;								// 结点的唯一键

		std::atomic<bool> _time_out_state = false;	// 是否超时

		std::atomic<bool> _dead_state = false;

		std::atomic<int> _not_ready_flag;			// 开始时与键值相同, 当为0时触发

		const int _time_out;						// 超时

		std::mutex _mtx;							// 当前结点的锁

		// 保存的数据
		std::map<KEY_TYPE, DATA_TYPE> _datas;

		// 保存的键值
		std::set<KEY_TYPE> _keys;

		// 返回的数据
		DATA_TYPE _out;

		// 向谁传递数据
		std::set<_thread_node_ptr<ID_TYPE, KEY_TYPE, DATA_TYPE>> _dest_nodes;

		// 计算方法
		node_mapping_func<KEY_TYPE, DATA_TYPE> _calc;

	protected:
		void _set_dead() {
#if CHECK_Thread
			auto tid = std::this_thread::get_id();
			printf("p%d Dead\n", tid);
#endif
			_dead_state = true;
		}

		// 当所有数据收到后, 设置为准备完毕, 这里的Ready是另外一个线程设置的
		void _check_ready() {
			if (--_not_ready_flag == 0) {
#if CHECK_Thread
				auto tid = std::this_thread::get_id();
				printf("p%d check Ready\n", tid);
#endif
			}
		}

		// 开始等待, 等待时启动计时器, 在结束的时候如果_not_ready_flag仍然为true, 就设置为死亡状态(after为0, 永不超时)
		void _wait(std::condition_variable& cv, std::unique_lock<std::mutex>& lock) {
#if CHECK_Thread
			auto tid = std::this_thread::get_id();
			printf("p%d Wait\n", tid);
#endif
			// 开始计时(session对象一直存在即可, 否则造成访问出错)
			if (_time_out != 0) {
				Timer().AsyncWait(_time_out, [this, &cv]() {
					if (this->_not_ready_flag && !this->_dead_state) {
						this->_dead_state = true;
						this->_not_ready_flag = 0;
						this->_time_out_state = true;
						cv.notify_all();
					}
				});
			}

			cv.wait(lock, [this]() { 	// 线程会一直在这里等待
#if CHECK_Thread
				auto tid = std::this_thread::get_id();
				printf("p%d notified", tid);
				printf(_not_ready_flag ? " but Sleep\n" : " Ready\n");
#endif
				return _not_ready_flag == 0;
			});
		}

		// 创建信号
		_Signal<ID_TYPE, KEY_TYPE, DATA_TYPE> _create_signal(const KEY_TYPE& key) {
			return _Signal<ID_TYPE, KEY_TYPE, DATA_TYPE>(key, _out);
		}

		// 向所有目标发送消息
		void _send_message_to_dest_and_die(std::condition_variable& cv, const KEY_TYPE& key) {
			auto tid = std::this_thread::get_id();
			if (!_dest_nodes.empty()) {
				for (auto& dest : _dest_nodes) {
#if CHECK_Thread
					printf("p%d Send message!\n", tid);
#endif
					assert(dest); // 说明dest为nullptr
					dest->_handle_message(cv, _create_signal(key));
				}
			}
#if CHECK_Thread
			else {
				printf("p%d dest is empty!\n", tid);
			}
#endif
			_set_dead();
		}

		// 数据是否合法 []][
		bool _is_vaild_data(const DATA_TYPE& data) const { return true; }

		// 键是自己所关心的信号键
		bool _is_vaild_key(const KEY_TYPE& key) const {
			return _keys.find(key) != _keys.end();
		}

		// 获取数据, 设置数据并判断是否有效, 尝试唤醒自己
		void _set_data(std::condition_variable& cv, const KEY_TYPE& key, const DATA_TYPE& data) {
			// 判断数据是否有效
			if (_is_vaild_key(key) && _is_vaild_data(data)) {
				_datas.insert_or_assign(key, data);
				// 获取数据之后, 标记设置为可运行
				_check_ready();
			}
#if CHECK_Thread
			auto tid = std::this_thread::get_id();
			printf("p%d tried notify_all\n", tid);
#endif
			cv.notify_all();	// 尝试唤醒线程, 若ready是true, 唤醒成功
		}

		// 获取信号, 根据key判断是否是自己关心的信号, 尝试唤醒自己
		void _handle_message(std::condition_variable& cv, const _Signal<ID_TYPE, KEY_TYPE, DATA_TYPE>& msg) {
			// 判断是否是自己关心的信号键
			if (_is_vaild_key(msg._key) && _is_vaild_data(msg._data)) {
#if CHECK_Thread
				auto tid = std::this_thread::get_id();
				printf("p%d vaild message!\n", tid);
#endif
				_datas.insert_or_assign(msg._key, msg._data);
				// 获取数据之后, 标记设置为可运行
				_check_ready();
			}
#if CHECK_Thread
			else {
				auto tid = std::this_thread::get_id();
				printf("p%d not vaild message!\n", tid);
			}
#endif
#if CHECK_Thread
			auto tid = std::this_thread::get_id();
			printf("p%d tried notify_all\n", tid);
#endif
			cv.notify_all();	// 尝试唤醒线程, 若ready是true, 唤醒成功
		}

	public:
		// 传递处理方法, 所有关心的信号键, 结点的id
		_Thread_Node(node_mapping_func<KEY_TYPE, DATA_TYPE> evalFunc, std::initializer_list<KEY_TYPE> keys, ID_TYPE id, int time_out = 0)
			:_calc(evalFunc), _keys(keys.begin(), keys.end()), _not_ready_flag(keys.size()), _id(id) , _time_out(time_out){ }

		// 添加目标
		void add_dest(_thread_node_ptr<ID_TYPE, KEY_TYPE, DATA_TYPE> task) {
			_dest_nodes.insert(task);
		}

		// 计算(线程安全)
		void eval(std::condition_variable& cv) {
			auto tid = std::this_thread::get_id();
			std::unique_lock<std::mutex> lock(_mtx);
			_wait(cv, lock);
			// 在没有死亡的时候执行计算, 发送消息
			if (!_dead_state) {
				// 执行计算
				KEY_TYPE k = _calc(_out, _datas, _keys);
				// 执行完毕, 向结点传递传递数据
				_send_message_to_dest_and_die(cv, k);
			}
#if CHECK_Thread
			else {
				if (_time_out_state) {
					auto tid = std::this_thread::get_id();
					printf("p%d Timeout\n", tid);
				}
			}
#endif
		}

		// 获取返回值的拷贝
		DATA_TYPE get_ret_data() {
			return DATA_TYPE(_out);
		}

		// 获取保存的值的拷贝
		DATA_TYPE get_data(KEY_TYPE key) {
			return _datas[key];
		}

		// 返回是否超时
		bool is_time_out() { return _time_out_state;	}

		// 检查状态
		bool is_dead() const { return _dead_state; }

		// 获取id
		ID_TYPE getID() const { return _id; }

	};

	// n1向目标n2请求数据
	template<class ID_TYPE, class KEY_TYPE, class DATA_TYPE>
	_thread_node_ptr<ID_TYPE, KEY_TYPE, DATA_TYPE> operator>>(_thread_node_ptr<ID_TYPE, KEY_TYPE, DATA_TYPE> n1, _thread_node_ptr<ID_TYPE, KEY_TYPE, DATA_TYPE> n2)
	{
		assert(n1 != nullptr && n2 != nullptr);
		n2->add_dest(n1);
		return n2;
	}

	// 会话对象
	template<class KEY_TYPE, class DATA_TYPE>
	class Session
	{
		template<class KEY_TYPE, class DATA_TYPE>friend class TaskGraphic;
	protected:
		std::condition_variable cv;					// 条件变量
		std::vector<std::thread> thread_pool;		// 线程池
		std::shared_ptr<TaskGraphic<KEY_TYPE, DATA_TYPE>> _graphic;	// 计算图

	protected:
		// 清空线程池
		void _clear_pool() {
			thread_pool.clear();
		}
	
	public:
		// 会话绑定计算图
		Session(std::shared_ptr<TaskGraphic<KEY_TYPE, DATA_TYPE>> graphic) : _graphic(graphic) {}

		// 开始计算(会等待会话结束, 主线程再继续运行)
		DATA_TYPE eval(vsTool::id_t first, KEY_TYPE key) {
			_graphic->_session_start(this, first);
			join_pool();				// 等待会话结束, 之后清空线程, 主线程再继续运行
			_clear_pool();
			auto node = _graphic->get_node(first);	// 返回结点值
			return node->get_ret_data();
		}

		// 同步线程池
		void join_pool() {
			// 同步所有的线程
			for (auto& t : thread_pool)
				t.joinable() && (t.join(), 0);
		}
	};

	// 计算图对象
	template<class KEY_TYPE, class DATA_TYPE>
	class TaskGraphic {
		template<class KEY_TYPE, class DATA_TYPE>friend class Session;

	protected:
		vsTool::GraphicBuilder<_thread_node_ptr<vsTool::id_t, KEY_TYPE, DATA_TYPE>> _builder;		// 线程图构建者
		vsTool::graphic_ptr<_thread_node_ptr<vsTool::id_t, KEY_TYPE, DATA_TYPE>> thread_node_g_ptr;	// 线程图
		std::map<vsTool::id_t, _thread_node_ptr<vsTool::id_t, KEY_TYPE, DATA_TYPE>> node_ptr_map;	// id -> node

		// 标记
		bool _end_called = false;	// 是否调用过end

	protected:

		// 获取线程结点指针(没有找到, 返回nullptr)
		_thread_node_ptr<vsTool::id_t, KEY_TYPE, DATA_TYPE> _get_thread_node(vsTool::id_t i) throw(std::exception) {
			auto ptr = node_ptr_map.find(i);
			return ptr != node_ptr_map.end() ? ptr->second : throw std::exception("Node not found!");
		}

		// 根据图保存的信息构建点
		void _build_graphic_of_thread() {
			thread_node_g_ptr->for_each_unordered(
				[](auto g, auto node) {
				auto task_ptr = node->get_data();
				auto vec = g->get_nodes_pointed_by(node->get_id());
				for (auto node : vec) {
					task_ptr >> node->get_data();
				}
				return true;
			});
		}

		// 根据会话请求分配线程, 不会启动不需要的结点
		void _alloc_thread(vsTool::id_t first, Session<KEY_TYPE, DATA_TYPE>* sess) {
			thread_node_g_ptr->for_each_dfs_norepeat(
				[this, sess](auto g, auto index) {
				auto ptr = g->get_node_ptr(index)->get_data();
				sess->thread_pool.emplace_back(&_Thread_Node<vsTool::id_t, KEY_TYPE, DATA_TYPE>::eval, ptr, std::ref(sess->cv));
				return true;
			}, [](auto g, auto index) { return true; }, first);
		}

		// 启动会话
		void _session_start(Session<KEY_TYPE, DATA_TYPE>* sess, vsTool::id_t index) {
			assert(_end_called);			// 必须调用end完成构建才能继续
			_alloc_thread(index, sess);		// 根据会话请求分配线程, 不会启动不需要的结点
		}

	public:
		TaskGraphic() : thread_node_g_ptr(nullptr), _builder(vsTool::BUILD_MODE::AUTO_MOVE) {}

		// 获取计算结点
		_thread_node_ptr<vsTool::id_t, KEY_TYPE, DATA_TYPE> get_node(vsTool::id_t index) throw(std::exception) {
			return _get_thread_node(index);
		}

		// 构建结点
		void build_node(node_mapping_func<KEY_TYPE, DATA_TYPE> evalFunc, const std::initializer_list<KEY_TYPE>& keys, vsTool::id_t id, int time_out = 0)
		{
			auto task_ptr = _thread_node_ptr<vsTool::id_t, KEY_TYPE, DATA_TYPE>
				(new _Thread_Node<vsTool::id_t, KEY_TYPE, DATA_TYPE>(evalFunc, keys, id, time_out));
			node_ptr_map.insert_or_assign(id, task_ptr);
		}

		// 构建图
		TaskGraphic<KEY_TYPE, DATA_TYPE>* head() throw(std::exception) {
			_builder.head(_get_thread_node(0));
			return this;
		}

		TaskGraphic<KEY_TYPE, DATA_TYPE>* gotoNode(vsTool::id_t index) throw(std::exception) {
			_builder.gotoNode(index);
			return this;
		}

		TaskGraphic<KEY_TYPE, DATA_TYPE>* linkToNode(vsTool::id_t index) throw(std::exception) {
			if (_builder.contain_node(index)) {
				_builder.linkToNode(index);
			}
			else {
				_builder.addNode(index, _get_thread_node(index));
			}
			return this;
		}

		// 根据图保存的信息构建计算图, 将builder初始化(规则: 如果遇到有环图, 会抛出异常[]][)
		void end() throw(std::exception) {
			thread_node_g_ptr = _builder.getGraphic();
			_build_graphic_of_thread();
			_builder.init();
			_end_called = true;
#if CHECK_Tool
			thread_node_g_ptr->show();
#endif
			if (thread_node_g_ptr->isLoop()) {
			//	throw std::exception("Graphic looped");
			}
		}

		// 初始化, 清空所有数据
		void init() {
			_builder.init();
			thread_node_g_ptr->init();
			node_ptr_map.clear();
		}
	};

}