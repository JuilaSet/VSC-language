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
	auto g = builder.head("c1")
		->addNode(1, "c2")
		->addNode(2, "c3")
		->getGraphic();
	g->show();

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
	TaskManager tm;
	tm.test_thread_pool();
	return 0;
}
