// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

#ifndef PCH_H
#define PCH_H

#define MODE_CIL false

#define CHECK_Data false

#define CHECK_Compiler false
#define CHECK_Compiler_alloc false
#define CHECK_Compiler_Field false
#define CHECK_Compiler_Field_NEW_BLK false

#define CHECK_Eval false
#define CHECK_Eval_command false

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

class IEvaluable;
class vsData;
class Command;
class vsEvaluator;
class vsVirtualMachine;
struct _StackFrame;
using vec_command_t = std::vector<Command>;
using data_ptr = std::shared_ptr<vsData>;
using evalable_ptr = std::shared_ptr<IEvaluable>;
using data_list_t = std::map<std::string, data_ptr>;
using new_data_list_t = std::vector<data_ptr>;
using RunTimeStackFrame = _StackFrame;
using RunTimeStackFrame_ptr = std::shared_ptr<_StackFrame>;

#include "Token.h"
#include "vsFrame.h"
#include "Data.h"

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
