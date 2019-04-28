#pragma once

class Activity {
protected:
	std::string _name;
	int _start_time;
	int _end_time;
public:
	// 随机分配时间
	static void initActivities_random_duration(Act_vec& activities, int seed = 255, int size = 10, const int max_time = 20);
	// 回显
	static void showActivities(Act_vec& activities, const int max_time);

public:
	Activity(const std::string& name, int start_time, int end_time) :_name(name), _start_time(start_time), _end_time(end_time) {}
	std::string getName() {
		return _name;
	}

	// 时间{不包括结尾: [start, end)}
	int getStartTime() {
		return _start_time;
	}
	int getEndTime() {
		return _end_time;
	}
	bool inDuration(int time) {
		return time >= getStartTime() && time < getEndTime();
	}
};

class Programe {
protected:
	std::string _name;
	Act_ptr activity;	// 对应处理程序
	bool used;			// 是否被使用过
protected:
	void _setAct(Act_ptr act_ptr) {
		activity = act_ptr;
	}

	// 显示处理过程
	void _showProcess() {
		std::cout << _name << " -> " << activity->getName() << std::endl;
	}
public:
	// 分配名称
	static void initPrograms(Pro_vec& programes, int size = 3);

	// 回显
	static void showPrograms(Pro_vec& activities);

public:
	Programe(const std::string& name) :_name(name), used(false), activity(nullptr) {}

	std::string getName() {
		return _name;
	}

	// 是否被使用过
	int isUsed() {
		return used;
	}

	// 任务
	Act_ptr getAct() {
		return activity;
	}

	// 处理活动
	void process(Act_ptr act) {
		used = true;
		_setAct(act);
		_showProcess();
	}

	// 正在处理(在处理时间内)
	bool isInProcess(int time) {
		return activity != nullptr && activity->inDuration(time);
	}

};

void allocProgram(Pro_vec& pros, Act_vec& acts, const int max_time);

int pro_used_count(Pro_vec& pros);
