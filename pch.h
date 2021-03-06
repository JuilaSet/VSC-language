﻿// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

#ifndef PCH_H
#define PCH_H

#define MODE_CIL false	
#define DEBUG_MODE true

#define CHECK_Tool false
#define CHECK_Thread false

#define CHECK_Data false
#define CHECK_Object false

#define CHECK_Lexer false
#define CHECK_Lexer_res false

#define CHECK_Parser false
#define CHECK_Parser_g false

#define CHECK_Compiler false
#define CHECK_Compiler_rev false
#define CHECK_Compiler_alloc false
#define CHECK_Compiler_Field false
#define CHECK_Compiler_Field_NEW_BLK false

#define CHECK_Compiler_Node false
#define CHECK_Compiler_Node_res false

#define CHECK_Eval false
#define CHECK_Eval_command false

// 字符串最大显示数量除数
const int const STRING_MAX_SZIE = 100;

// 未定义时回显字符串
const char* const UNDEFINED_ECHO = "undefined";
 
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
#include <atomic>
#include <condition_variable>

// 调试
#include <cstdlib>
#include <cassert>
#include <exception>
#include <cstdio>

class IEvaluable;
class vsData;

class vsContainer;
class vsLinearContainer;
class vsIdentifierContainer;

class Command;
class vsblock_static;
class vsEvaluator;
class vsVirtualMachine;
enum class WordType;

struct _StackFrame;

using vec_command_t = std::vector<Command>;
using data_ptr = std::shared_ptr<vsData>;
using evalable_ptr = std::shared_ptr<IEvaluable>;
using block_ptr = std::shared_ptr<vsblock_static>;
using index_t = long int;

// compiler
using word_type_map = std::map<std::string, WordType>;
using local_index_set = std::set<std::string>;			// 局部变量索引
using form_paras_set = std::set<std::string>;			// 存放形参名集合
using form_paras_vec = std::vector<std::string>;		// 形参表
// using act_paras_vec = std::vector<data_ptr>;
// using new_data_list_t = std::vector<data_ptr>;
// using act_paras_list_t = std::map<std::string, data_ptr>;// 形参结合表(函数内部解引用使用)

// eval
using container_ptr = std::shared_ptr<vsContainer>;		 // 容器指针
using form_paras_list_t = std::vector<std::string>;		 // 形参列表(block编译时确定)
using pass_paras_list_t = std::vector<data_ptr>;		 // 传递参数的列表(call时调用)
// using data_list_t = std::map<std::string, data_ptr>;		局部变量表
using local_var_container_ptr = container_ptr;			 // 局部变量表
using para_var_container_ptr = container_ptr;			 // 形参结合表
using RunTimeStackFrame = _StackFrame;					 // 运行时栈帧
using RunTimeStackFrame_ptr = std::shared_ptr<_StackFrame>;

// tool
#include "Graphic.h"
#include "vsThread.h"
// 
#include "Token.h"
#include "vsFrame.h"
#include "Data.h"
#include "vsObject.h"
#include "vsContainer.h"
#include "Node.h"
#include "Input.h"
#include "Lexer.h"
#include "Parser.h"
#include "vsEvaluator.h"
#include "vsoperator.h"
#include "vscommand.h"
#include "vsfunction.h"
#include "Adaptor.h"
#include "Compiler.h"
#include "vsVirtualMachine.h"
#include "vsFrontend.h"

#endif //PCH_H
