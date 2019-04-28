#include "pch.h"
#include "activity.h"

// 随机分配时间(可能有空任务)
void Activity::initActivities_random_duration(Act_vec& activities, int seed, int size, const int max_time) {

	std::default_random_engine e;							// 随机引擎
	std::uniform_int_distribution<int> id(0, max_time / 2);	// 分布变量

	e.seed(seed);
	for (int i = 0; i < size; ++i) {
		auto start_time = id(e);
		auto end_time = start_time + id(e) + 1;	// [0, MAX_TIME / 2) + [0, MAX_TIME / 2) = [0, MAX_TIME)
		activities.emplace_back(new Activity("Act" + std::to_string(i), start_time, end_time));
	}
}

// 回显
void Activity::showActivities(Act_vec& activities, const int max_time) {
	std::cout << std::endl << "Activities: " << std::endl;
	auto begin = activities.begin();
	auto end = activities.end();
	for (auto it = begin; it != end; ++it) {
		auto name = (*it)->getName();
		auto start_time = (*it)->getStartTime();
		auto end_time = (*it)->getEndTime();
		std::cout << name << " : ";
		for (int time = 0; time < max_time; ++time) {
			if ((*it)->inDuration(time))
				std::cout << '+';
			else
				std::cout << '-';
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

// 分配名称
void Programe::initPrograms(Pro_vec& programes, int size) {
	std::stringstream ss;
	for (int i = 0; i < size; ++i) {
		programes.emplace_back(new Programe("Pro" + std::to_string(i)));
	}
}

// 回显
void Programe::showPrograms(Pro_vec& activities) {
	std::cout << std::endl << "Programes: " << std::endl;
	auto begin = activities.begin();
	auto end = activities.end();
	for (auto it = begin; it != end; ++it) {
		std::cout << (*it)->getName() << std::ends;
	}
	std::cout << std::endl << std::endl;
}

// 取出一个闲置的程序(如果没有, 取出nullptr)
Pro_ptr getIdlePrograme(Pro_vec& pros, int time) {
	for (auto idle_pro : pros) {
		if (!idle_pro->isInProcess(time)) {
			return idle_pro;
		}
	}
	return nullptr;
}

// 为活动分配处理程序
void allocProgram(Pro_vec& pros, Act_vec& acts, const int max_time) {
	// 模拟时间走动
	for (int time = 0; time < max_time; ++time) {
		for (auto act : acts) {
			// 如果在时间内
			if (act->getStartTime() == time) {
				std::cout << time << ":\t";
				// 抓住一个空闲程序
				auto idle = getIdlePrograme(pros, time);
				if (idle == nullptr)
					std::cout << "没有可以分配的处理程序给 " << act->getName() << "使用" << std::endl;
				else
					// 为这个活动分配处理程序
					idle->process(act);
			}
		}
	}
	std::cout << std::endl;
}

// 统计使用个数
int pro_used_count(Pro_vec& pros) {
	int used_pro_count = 0;
	for (auto pro : pros)
		if (pro->isUsed())
			used_pro_count++;
	return used_pro_count;
}
