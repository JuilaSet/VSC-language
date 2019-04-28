// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

#ifndef PCH_H
#define PCH_H

#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <random>
#include <queue>
#include <initializer_list>
#include <cassert>
#include <set>
#include <functional>
#include <algorithm>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <atomic>

class Activity;
class Programe;
using Act_ptr = std::shared_ptr<Activity>;
using Pro_ptr = std::shared_ptr<Programe>;
using Act_vec = std::vector<Act_ptr>;
using Pro_vec = std::vector<Pro_ptr>;

#include "activity.h"
#include "Graphic.h"
#include "vsThread.h"

#endif // PCH_H
