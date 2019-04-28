// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
using namespace std;

void test_activity() {
	const int MAX_TIME = 10;

	Pro_vec pros;
	Act_vec acts;
	// 设置4个任务
	Activity::initActivities_random_duration(acts, 4, 8, MAX_TIME);
	Activity::showActivities(acts, MAX_TIME);
	// 设置3个处理程序
	Programe::initPrograms(pros, 10);
	Programe::showPrograms(pros);
	// 分布式处理开始
	allocProgram(pros, acts, MAX_TIME);
	// 统计使用个数
	cout << "处理程序的使用个数: " << pro_used_count(pros) << endl;
}

void test_graphic() {
	vsTool::GraphicBuilder<string> builder(vsTool::BUILD_MODE::AUTO_MOVE);
	auto g = builder.head("c0")
		->addNode(1, "c1")
		->addNode(2, "c2")
		->addNode(3, "c3")
		->gotoNode(3)
		->linkToNode(1)
		->gotoNode(0)
		->linkToNode(3)
		->getGraphic();
	g->show();

	g->for_each_dfs_norepeat([](auto g, auto index) {
		cout << g->get_node_ptr(index)->get_data() << endl;
		return true; 
	}, [](auto g, auto index) {return true; }, 0);

	cout << g->isDirected() << endl;
	cout << g->isLoop() << endl;
}

void test_dfa() {
	vsTool::StateMachineBuilder<std::string, std::string> builder;
	auto dfa = builder.
		head("born")->addNode(1, "Thank you !", "eat")->linkToNode(1, "eat") // eat没有加进去
		->gotoNode(0)->addNode(2, "...die...", "kill")
		->gotoNode(0)->addNode(3, "I'm sad", "hurt")->addNode(4, "Don't hurt me!","hurt")
		->linkToNode(3, "eat")->linkToNode(1, "eat")
		->gotoNode(1)->linkToNode(2, "kill")
		->gotoNode(1)->linkToNode(3, "hurt")
		->gotoNode(4)->linkToNode(2, "hurt")
		->gotoNode(4)->linkToNode(2, "kill")
		->gotoNode(3)->linkToNode(2, "kill")
		->gotoNode(2)->linkToNode(0, "reborn")
		->getDFA();

	std::cout << "Hello I'm " << dfa->get_cur_state()->get_data() << std::endl;
	std::string sit;
	while (std::cout << "->", std::cin >> sit) {
		dfa->state_input(sit);
		std::cout << dfa->get_cur_state()->get_data() << std::endl;
	}
}

int main() {
	vsThread::taskGraphic_ptr<string, int> tm(new vsThread::TaskGraphic<string, int>);

	// 根据设置初始化结点信息
	tm->build_node([](auto& _ret_data, auto& datas, auto& keys) {
		auto tid = std::this_thread::get_id();
		_ret_data = datas["x"] + 1;
		for (int i = 0; i < 3; ++i) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("p%d clac0: %d\n", tid, int(_ret_data));
		}
		return "x";
	}, { "x" }, 0);
	tm->build_node([](auto& _ret_data, auto& datas, auto& keys) {
		auto tid = std::this_thread::get_id();
		_ret_data = datas["x"] + 1;
		for (int i = 0; i < 3; ++i) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("p%d clac1: %d\n", tid, int(_ret_data));
		}
		return "x";
	}, { "x" }, 1);
	tm->build_node([](auto& _ret_data, auto& datas, auto& keys) {
		auto tid = std::this_thread::get_id();
		int d;
		std::cin >> d;
		_ret_data = d;
		for (int i = 0; i < 3; ++i) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("p%d clac2: %d\n", tid, int(_ret_data));
		}
		return "x";
	}, { }, 2);
	tm->build_node([](auto& _ret_data, auto& datas, auto& keys) {
		auto tid = std::this_thread::get_id();
		_ret_data = datas["x"] + 1;
		for (int i = 0; i < 3; ++i) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("p%d clac3: %d\n", tid, int(_ret_data));
		}
		return "x";
	}, { "x" }, 3);
	tm->build_node([](auto& _ret_data, auto& datas, auto& keys) {
		auto tid = std::this_thread::get_id();
		_ret_data = 100;
		for (int i = 0; i < 3; ++i) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("p%d clac4: %d\n", tid, int(_ret_data));
		}
		return "x";
	}, { }, 4);

	// 构建图信息
	tm->head()->linkToNode(1)->linkToNode(2)->linkToNode(3)
		->gotoNode(1)->linkToNode(3)
		->gotoNode(0)->linkToNode(3)//->linkToNode(4)
		->end();

	vsThread::Session<std::string, int> sess(tm);
	sess.eval(1, "x");
	printf("Session end!\n");
	return 0;
}
