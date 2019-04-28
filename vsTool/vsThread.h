#pragma once

#define CHECK_Thread true

template<class ID_TYPE, class KEY_TYPE, class DATA_TYPE>
class _Thread_Node;

template<class ID_TYPE, class KEY_TYPE, class DATA_TYPE>
using _thread_node_ptr = std::shared_ptr<_Thread_Node<ID_TYPE, KEY_TYPE, DATA_TYPE>>;

// 返回信号键
template<class KEY_TYPE, class DATA_TYPE>
using node_mapping_func = std::function<KEY_TYPE(std::atomic<DATA_TYPE>&, std::map<KEY_TYPE, DATA_TYPE>&, std::set<KEY_TYPE>&)>;

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
	_Signal(KEY_TYPE key, DATA_TYPE data): _key(key), _data(data){	}
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

// 任务类(规则: ID_TYPE必须是可赋值的, KEY_TYPE必须是可赋值 /\ 可比较的, DATA_TYPE必须有空对象 /\ 必须是可赋值的)
template<class ID_TYPE, class KEY_TYPE, class DATA_TYPE>
class _Thread_Node {
protected:
	ID_TYPE _id;						// 结点的唯一键

	std::atomic<bool> _dead_state = false;

	int _not_ready_flag;				// 开始时与键值相同, 当为0时触发

	std::mutex _mtx;					// 当前结点的锁

	// 保存的数据
	std::map<KEY_TYPE, DATA_TYPE> _datas;

	// 保存的键值
	std::set<KEY_TYPE> _keys;

	// 返回的数据
	std::atomic<DATA_TYPE> _ret_data;

	// 向谁传递数据
//	std::set<ID_TYPE> _dest_nodes;
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

	void _wait(std::condition_variable& cv, std::unique_lock<std::mutex>& lock) {
#if CHECK_Thread
		auto tid = std::this_thread::get_id();
		printf("p%d Wait\n", tid);
#endif
		cv.wait(lock, [this, tid]() { 	// 线程会一直在这里等待
#if CHECK_Thread
			printf("p%d notified", tid);
			printf(_not_ready_flag ? " but Sleep\n" : " Ready\n");
#endif
			return _not_ready_flag == 0;
		});
	}

	// 创建信号
	_Signal<ID_TYPE, KEY_TYPE, DATA_TYPE> _create_signal(const KEY_TYPE& key) {
		return _Signal<ID_TYPE, KEY_TYPE, DATA_TYPE>(key, _ret_data);
	}

	// 向所有目标发送消息
	void _send_message_to_dest_and_die(std::condition_variable& cv, const KEY_TYPE& key){
		auto tid = std::this_thread::get_id();
		if (!_dest_nodes.empty()) {
			for (auto& dest : _dest_nodes) {
#if CHECK_Thread
				printf("p%d Send message!\n", tid);
#endif
				dest->handle_message(cv, _create_signal(key));
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
		std::cout << "Keys: ";
		for (auto k : _keys) {
			std::cout << k << std::ends;
		}
		std::cout << std::endl;
		return _keys.find(key) != _keys.end(); 
	}

public:
	// 传递处理方法, 所有关心的信号键, 结点的id
	_Thread_Node(node_mapping_func<KEY_TYPE, DATA_TYPE> evalFunc, std::initializer_list<KEY_TYPE> keys, ID_TYPE id)
		:_calc(evalFunc), _keys(keys.begin(), keys.end()), _not_ready_flag(keys.size()), _id(id) { }

	// 添加目标
	void add_dest(_thread_node_ptr<ID_TYPE, KEY_TYPE, DATA_TYPE> task) {
		_dest_nodes.insert(task);
	}

	// 计算(线程安全)
	void eval(std::condition_variable& cv) {
		auto tid = std::this_thread::get_id();
		std::unique_lock<std::mutex> lock(_mtx);
		_wait(cv, lock);
		// 执行计算
		KEY_TYPE k = _calc(_ret_data, _datas, _keys);
		// 执行完毕, 向结点传递传递数据
		_send_message_to_dest_and_die(cv, k);
	}

	// 获取数据, 设置数据并判断是否有效, 尝试唤醒自己
	void set_data(std::condition_variable& cv, const KEY_TYPE& key, const DATA_TYPE& data) {
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
	void handle_message(std::condition_variable& cv, const _Signal<ID_TYPE, KEY_TYPE, DATA_TYPE>& msg) {
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

	// 检查状态
	bool is_dead() const {	return _dead_state;	}

	// 获取id
	ID_TYPE getID() const { return _id; }

};

// n1向目标n2请求数据
template<class ID_TYPE, class KEY_TYPE, class DATA_TYPE>
_thread_node_ptr<ID_TYPE, KEY_TYPE, DATA_TYPE> operator>>(_thread_node_ptr<ID_TYPE, KEY_TYPE, DATA_TYPE> n1, _thread_node_ptr<ID_TYPE, KEY_TYPE, DATA_TYPE> n2)
{
	n2->add_dest(n1);
	return n2;
}

class TaskManager {
protected:
	std::condition_variable cv;		// 条件变量

	std::vector<std::thread> thread_pool;

protected:
	void _join_pool() {
		// 同步所有的线程
		for (auto& t : thread_pool)
			t.joinable() && (t.join(), 0);
	}

	template<class ID_TYPE, class KEY_TYPE, class DATA_TYPE>
	void _run(_thread_node_ptr<ID_TYPE, KEY_TYPE, DATA_TYPE> task_ptr) {
		thread_pool.emplace_back(&_Thread_Node<ID_TYPE, KEY_TYPE, DATA_TYPE>::eval, task_ptr, std::ref(cv));
	}

public:
	// 测试线程池
	void test_thread_pool() {
		auto task0_ptr = _thread_node_ptr<int, std::string, int>(new _Thread_Node<int, std::string, int>(
			[](auto& _ret_data, auto& datas, auto& keys) {
			auto tid = std::this_thread::get_id();
			for (int i = 0; i < 5; ++i) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				printf("p%d clac: %d\n", tid, datas["z"]);
			}
			_ret_data = 2;
			return "x";
		}, { "z" }, 0));

		auto task1_ptr = _thread_node_ptr<int, std::string, int>(new _Thread_Node<int, std::string, int>(
			[](auto& _ret_data, auto& datas, auto& keys) {
				auto tid = std::this_thread::get_id();
				for (int i = 0; i < 5; ++i) {
					std::this_thread::sleep_for(std::chrono::seconds(1));
					printf("p%d clac: %d\n", tid, datas["x"]);
				}
				_ret_data = 1;
				return "a";
			}, { "x" }, 1));

		auto task2_ptr = _thread_node_ptr<int, std::string, int>(new _Thread_Node<int, std::string, int>(
			[](auto& _ret_data, auto& datas, auto& keys) {
				auto tid = std::this_thread::get_id();
				for (int i = 0; i < 5; ++i) {
					std::this_thread::sleep_for(std::chrono::seconds(1));
					printf("p%d clac: %d %d\n", tid, datas["x"], datas["y"]);
				}
				_ret_data = 2;
				return "a";
			}, { "x", "y" }, 2));

		auto task3_ptr = _thread_node_ptr<int, std::string, int>(new _Thread_Node<int, std::string, int>(
			[](auto& _ret_data, auto& datas, auto& keys) {
				auto tid = std::this_thread::get_id();
				for (int i = 0; i < 5; ++i) {
					std::this_thread::sleep_for(std::chrono::seconds(1));
					printf("p%d clac: %d\n", tid, datas["y"]);
				}
				_ret_data = 3;
				return "a";
			}, { "y" }, 3));

		auto task4_ptr = _thread_node_ptr<int, std::string, int>(new _Thread_Node<int, std::string, int>(
			[](auto& _ret_data, auto& datas, auto& keys) {
			auto tid = std::this_thread::get_id();
			for (int i = 0; i < 5; ++i) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				printf("p%d clac: %d\n", tid, datas["z"]);
			}
			_ret_data = 4;
			return "y";
		}, { "z" }, 4));

		auto taska_ptr = _thread_node_ptr<int, std::string, int>(new _Thread_Node<int, std::string, int>(
			[](auto& _ret_data, auto& datas, auto& keys) {
			auto tid = std::this_thread::get_id();
			for (int i = 0; i < 5; ++i) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				printf("p%d clac: %d\n", tid, datas["z"]);
			}
			_ret_data = -1;
			return "";
		}, { "a" }, 5));

		task1_ptr >> task0_ptr;
		task2_ptr >> task0_ptr;
		task2_ptr >> task4_ptr;
		task3_ptr >> task0_ptr;
		task3_ptr >> task4_ptr;
		// end
		taska_ptr >> task1_ptr;
		taska_ptr >> task2_ptr;
		taska_ptr >> task3_ptr;

		_run(task0_ptr);
		_run(task1_ptr);
		_run(task2_ptr);
		_run(task3_ptr);
		_run(task4_ptr);
		_run(taska_ptr);

		// 主线程中对数据进行输入
		int i;
		while (std::cin >> i) {
			task0_ptr->set_data(cv, "z", i);
			task4_ptr->set_data(cv, "z", i);
			if (taska_ptr->is_dead()) {
				printf("Main Session Finished\n");
				break;
			}
		}
	}

	~TaskManager() {
		_join_pool();
	}
};
