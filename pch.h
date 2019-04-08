// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

#ifndef PCH_H
#define PCH_H

// 最大栈帧数, 超过会触发栈溢出异常
#define MAX_STACK_SIZE 1024

// 分配给标识符的索引最大量
#define MAX_SLOT_SIZE 256

// 分配给block的最大数量
#define MAX_BLOCK_INDEX_SIZE 1024

// 流
#include <iostream>
#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>

// 容器
#include <functional>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <stack>
#include <initializer_list>
#include <set>

// 线程
#include <memory>
#include <chrono>
#include <thread>

// 调试
#include <cstdlib>
#include <cassert>
#include <exception>

#include "Token.h"
#include "Data.h"
#include "vsObject.h"

#include "Input.h"
#include "Lexer.h"
#include "Parser.h"
#include "vsEvaluator.h"
#include "vsoperator.h"
#include "vscommand.h"
#include "vsfunction.h"
#include "Compiler.h"
#include "vsVirtualMachine.h"

#endif //PCH_H
