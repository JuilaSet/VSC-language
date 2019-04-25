// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"

using namespace std;

// 注册关键字到上下文的关联
void regist_keywords_contents(Context_helper& helper) {

	// CONTROLLER
	
	helper.regist_context(Word(WordType::CONTROLLER, "lambda").serialize(),
		helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
			compiler->def_paras_begin();
			compiler->dont_gene_callblk();
			compiler->setSubFieldStrongHold();
			return _command_set{ };
		},
		{
			// ALWAYS实现了不定参数
			Context([=](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				return _command_set{  };
			}, Context_Type::ALWAYS),
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				compiler->def_paras_end();
				compiler->enable_gene_callblk();
				compiler->setSubFieldWeakHold();
				return _command_set{ };
			}, Context_Type::END)
		})
	); 

	helper.regist_context(Word(WordType::CONTROLLER, "enclosed").serialize(),
		helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w, auto* compiler) {
		compiler->def_paras_begin();
		compiler->dont_gene_callblk();
		compiler->setSubFieldWeakHold();
		return _command_set{ };
	},
		{
			// ALWAYS实现了不定参数
			Context([=](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				return _command_set{ };
			}, Context_Type::ALWAYS),
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				compiler->def_paras_end();
				compiler->enable_gene_callblk();
				compiler->setSubFieldWeakHold();
				return _command_set{ COMMAND(ENCLOSED) };
			}, Context_Type::END)
		})
	);
	
	/*
	helper.regist_context(Word(WordType::CONTROLLER, "prcd").serialize(),
		helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
			helper.push_command_index(_vec.size());
			return _command_set{
					COMMAND(ERROR),	// 待确定
					COMMAND(JMP)
			};
		},
		{
			Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
				return _command_set{ };
			}, Context_Type::NORMAL, "prcd_op1"),
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				int last_index = helper.pop_command_index();
				int addr = _vec.size();
				_vec[last_index] = CommandHelper::getPushOpera(data_ptr(new NumData(DataType::OPERA_ADDR, addr + 1)));
				return _command_set{
						COMMAND(RET),
						CommandHelper::getPushOpera(data_ptr(new NumData((DataType::OPERA_ADDR, last_index + 2)))	// push过程的首地址
				};
			}, Context_Type::END, "prcd_end")
		})
	);
	*/

	helper.regist_context(Word(WordType::CONTROLLER, "def").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
				compiler->in_def();
				return _command_set{};
			},
			{
				Context([&](ContextStk& cstk, _command_set& commandVec, Word& w,  auto* compiler) {
					compiler->out_def();
					compiler->dont_gene_callblk();
					// 获取分配的下标
					compiler->insert_local(w, WordType::IDENTIFIER);	// 告知Compiler声明过了第一个参数
//					commandVec.pop_back(); // 将之前的 push 立即数指令弹出 []][
//					commandVec.push_back(CommandHelper::getPushOpera(data_ptr(new IndexData(DataType::ID_INDEX, w.getString()))));
					compiler->setSubFieldStrongHold();
					return _command_set{};
				}, Context_Type::NORMAL, "def_op1"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->enable_gene_callblk();
					return _command_set{};
				}, Context_Type::NORMAL, "def_op2"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->setSubFieldWeakHold();
					return _command_set{
						COMMAND(NEW_DEF)
					};
				}, Context_Type::END, "def_end")
			})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "assign").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {
				compiler->in_def();
				return _command_set{};
			},
			{
				Context([&](ContextStk& cstk, _command_set& commandVec, Word& w,  auto* compiler) {
					compiler->out_def();
					compiler->dont_gene_callblk();
					compiler->setSubFieldStrongHold();
					// 检查word是否定义: 如果当前只有一个word作为标识符 []][
					if (w.getType() == WordType::IDENTIFIER_ENABLED && !compiler->has_local(w) && !compiler->has_para(w)) {
						throw undefined_exception(w.getString());
					}
					return _command_set{};
				}, Context_Type::NORMAL, "assign_op1"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->enable_gene_callblk();
					return _command_set{};
				}, Context_Type::NORMAL, "assign_op2"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->setSubFieldWeakHold();
					return _command_set{
						// COMMAND(ASSIGN)
						COMMAND(NEW_ASSIGN)
					};
				}, Context_Type::END, "assign_end")
			})
	);
	
	helper.regist_context(Word(WordType::CONTROLLER, "cp").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {
				compiler->in_def();
				return _command_set{};
			},
			{
				Context([&](ContextStk& cstk, _command_set& commandVec, Word& w,  auto* compiler) {
					compiler->out_def();
					compiler->dont_gene_callblk();
					compiler->setSubFieldStrongHold();
					// 检查word是否定义: 如果当前只有一个word作为标识符 []][
					if (w.getType() == WordType::IDENTIFIER_ENABLED && !compiler->has_local(w) && !compiler->has_para(w)) {
							throw undefined_exception(w.getString());
					}
					return _command_set{};
				}, Context_Type::NORMAL, "copy_op1"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->enable_gene_callblk();
					return _command_set{};
				}, Context_Type::NORMAL, "copy_op2"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->setSubFieldWeakHold();
					return _command_set{
						COMMAND(CP)
					};
				}, Context_Type::END, "copy_end")
			})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "break").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w, auto* compiler) { return _command_set{}; },
			{
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{
						COMMAND(BREAK)
					};
				}, Context_Type::END)
			})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "return").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w, auto* compiler) { 
				compiler->dont_gene_callblk();
				compiler->setSubFieldStrongHold();
				return _command_set{};
			},
			{
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->enable_gene_callblk();
					compiler->setSubFieldWeakHold();
					return _command_set{
						COMMAND(RET)
					};
				}, Context_Type::END)
			})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "abort").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) { return _command_set{}; },
			{
				// 转为数字
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_1
					if (w.getType() != WordType::NUMBER) {
						return _command_set{
							COMMAND(CAST_NUMBER)
						};
					}
					else {
						return _command_set{ };
					}
				}, Context_Type::NORMAL, "abort_op1"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{
						COMMAND(ABORT)
					};
				}, Context_Type::END)
			})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "ignore").serialize(),
		helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
			helper.push_command_index(_vec.size());
			return _command_set{
					COMMAND(ERROR),	// 待确定
					COMMAND(JMP)
			};
		},
		{
			EMPTY_CONTEXT,
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				int last_index = helper.pop_command_index();
				int addr = _vec.size();
				_vec[last_index] = CommandHelper::getPushOpera(data_ptr(new AddrData(addr)));
				return _command_set{ };
			}, Context_Type::END)
		})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "if").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) { return _command_set{}; },
			{
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					helper.push_command_index(_vec.size() + 1);
					return _command_set{
						COMMAND(TEST),
						COMMAND(ERROR),		// 待确定
						COMMAND(JMP_FALSE)
					};
				}, Context_Type::NORMAL, "if_p1"),
				EMPTY_CONTEXT,
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					int last_index = helper.pop_command_index();
					int addr = _vec.size();
					_vec[last_index] = CommandHelper::getPushOpera(data_ptr(new AddrData(addr)));
					return _command_set{ };
				}, Context_Type::END, "if_end")
			})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "while").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				helper.push_command_index(_vec.size());
				return _command_set{};
			},
			{
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					helper.push_command_index(_vec.size() + 1);
					return _command_set{
						COMMAND(TEST),
						COMMAND(ERROR),		// 待确定
						COMMAND(JMP_FALSE)
					};
				}),
				EMPTY_CONTEXT,
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					int last_index = helper.pop_command_index();
					int head_index = helper.pop_command_index();
					int addr = _vec.size();
					_vec[last_index] = CommandHelper::getPushOpera(data_ptr(new AddrData(addr + 2)));
					return _command_set{
						CommandHelper::getPushOpera(data_ptr(new AddrData(head_index))),
						COMMAND(JMP)
					};
				}, Context_Type::END)
			})
	);

	/*
	helper.regist_context(Word(WordType::CONTROLLER, "rept").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {	return _command_set{}; },
			{
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					helper.push_command_index(_vec.size() + 1);
					return _command_set {
						COMMAND(COUNT),
						COMMAND(NOP)
					};
				}, Context_Type::NORMAL, "rept_op1"),
				// block
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{ };
				}, Context_Type::NORMAL, "rept_block"),
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					int addr = helper.pop_command_index();
					return _command_set{
						CommandHelper::getPushOpera(NumData(DataType::OPERA_ADDR, addr)),
						COMMAND(REPT)};
				}, Context_Type::END, "rept_end")
			})
	);
	*/
	
	// IDENTIFIER_SPEC
	
	helper.regist_context(Word(WordType::IDENTIFIER_SPEC, "$C").serialize(),
		helper.build_context(
		[](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_0
#if CHECK_Compiler
		cout << "COMMAND:::: $C" << endl;
#endif
			return _command_set{ COMMAND(ECX) };
		}, { })		// 只能有一个上下文
	);

	helper.regist_context(Word(WordType::IDENTIFIER_SPEC, "$null").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_0
#if CHECK_Compiler
		cout << "COMMAND:::: $null" << endl;
#endif
			return _command_set{ COMMAND(NUL) };
		}, { })	// 只能有一个上下文
	);

	helper.regist_context(Word(WordType::IDENTIFIER_SPEC, "$vec").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {
#if CHECK_Compiler
					cout << "COMMAND:: $vec" << endl;
#endif
					return _command_set{ 
						CommandHelper::getPushOpera(data_ptr(new vsVector))
					};
				}, { })	// 只能有一个上下文
			);


	helper.regist_context(Word(WordType::IDENTIFIER_SPEC, "$obj").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {
#if CHECK_Compiler
		cout << "COMMAND:: $obj" << endl;
#endif
				return _command_set{
					CommandHelper::getPushOpera(data_ptr(new vsOriginObject))
				};
			}, { })	// 只能有一个上下文
		);
	// OPERATOR_WORD

	helper.regist_context(Word(WordType::OPERATOR_WORD, "time").serialize(),
		helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w, auto* compiler) {
		helper.push_command_index(_vec.size());
		return _command_set{
				COMMAND(TIME_BEGIN)
		};
	},
		{
			Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
				return _command_set{ };
			}, Context_Type::NORMAL, "time_block"),
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				return _command_set{
						COMMAND(TIME_END)
				};
			}, Context_Type::END, "time_end")
		})
	);

	helper.regist_context(Word(WordType::OPERATOR_WORD, "call").serialize(),
		helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w, auto* compiler) {
		compiler->paras_begin();
		compiler->dont_gene_callblk();
		return _command_set{ COMMAND(CALL_BLK_BEGIN) };
	},
		{
			Context([=](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				compiler->paras()++;
				compiler->enable_gene_callblk();
				return _command_set{
					COMMAND(PARA_PASS)
				};
			}, Context_Type::ALWAYS),
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				auto count = compiler->paras();
				compiler->paras_end();
				return _command_set{
						CommandHelper::getPushOpera(data_ptr(new NumData(count))),
						COMMAND(CALL_BLK)
				};
			}, Context_Type::END)
		})
	);

	helper.regist_context(Word(WordType::OPERATOR_WORD, "in").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {
				compiler->save_def();
				return _command_set{};
			},
			{
				Context([&](ContextStk& cstk, _command_set& commandVec, Word& w,  auto* compiler) {
					compiler->in_def();
					return _command_set{};
				}, Context_Type::NORMAL, "in_op1"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->out_def();
					return _command_set{
						COMMAND(IN)
					};
				}, Context_Type::NORMAL, "in_op2"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->reserve_def();
					return _command_set{};
				}, Context_Type::END, "in_end")
			})
	);

	helper.regist_context(Word(WordType::OPERATOR_WORD, "add").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_0
				return _command_set{ };
			},
			{
				// 转为数字
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_1
					return _command_set{ };
				}, Context_Type::NORMAL, "add_op1"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_2
						return _command_set{ };
				}, Context_Type::NORMAL, "add_op2"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_end
					return _command_set{
						COMMAND(ADD)
					};
				}, Context_Type::END, "add_end")
			})
	);

	helper.regist_context(Word(WordType::OPERATOR_WORD, "typename").serialize(),
		helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w, auto* compiler) {
		helper.push_command_index(_vec.size());
			return _command_set{ };
		},
		{
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				return _command_set{ COMMAND(TYPENAME) };
			}, Context_Type::NORMAL, "typename_op1"),
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				return _command_set{ };
			}, Context_Type::END, "typename_end")
		})
	);

	/*

	helper.regist_context(Word(WordType::OPERATOR_WORD, "sub").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
				return _command_set{ };
			},
			{
				// 转为数字
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_1
					if (w.getType() != WordType::NUMBER) {
						return _command_set{
							COMMAND(CAST_NUMBER)
						};
					}
					else {
						return _command_set{ };
					}
				}),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_2
					if (w.getType() != WordType::NUMBER) {
						return _command_set{
							COMMAND(CAST_NUMBER)
						};
					}
					else {
						return _command_set{ };
					}
				}),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{
						COMMAND(SUB)
					};
				}, Context_Type::END)
			})
	);

	helper.regist_context(Word(WordType::OPERATOR_WORD, "not").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
				return _command_set{ };
			},
			{
				// 转为数字
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_1
					return _command_set{
						COMMAND(CAST_NUMBER)
					};
				}),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{
						COMMAND(NOT)
					};
				}, Context_Type::END)
			})
	);

	helper.regist_context(Word(WordType::OPERATOR_WORD, "eq").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
				return _command_set{ };
			},
			{
				EMPTY_CONTEXT,
				EMPTY_CONTEXT,
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{
						COMMAND(EQ)
					};
				}, Context_Type::END)
			})
	);*/

	/*
	helper.regist_context(Word(WordType::OPERATOR_WORD, "l").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
				return _command_set{ };
			},
			{
				EMPTY_CONTEXT,
				EMPTY_CONTEXT,
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{
						COMMAND(L)
					};
				}, Context_Type::END)
			})
	);

	helper.regist_context(Word(WordType::OPERATOR_WORD, "g").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
				return _command_set{ };
			},
			{
					EMPTY_CONTEXT,
					EMPTY_CONTEXT,
					Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
						return _command_set{
							COMMAND(G)
						};
					}, Context_Type::END)
				})
	);
	*/

	helper.regist_context(Word(WordType::OPERATOR_WORD, "echo").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
				compiler->dont_gene_callblk(); 
				return _command_set{}; 
			},
			{
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
#if CHECK_Compiler
					cout << "COMMAND ECHO" << endl;
#endif
					return _command_set{
						CommandHelper::getEchoOpera(&cout)
					};
				}, Context_Type::ALWAYS, "echo_op*"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->enable_gene_callblk();
					return _command_set{ };
				}, Context_Type::END, "echo_end")
			})
	);
}

// 注册各种符号
void regist_token(Token_helper& helper) {
	// 必须项目
	REGIST_TOKEN(helper, "/*", "commet_open");
	REGIST_TOKEN(helper, "*/", "commet_close");
	REGIST_TOKEN(helper, "|<", "cstring_open");
	REGIST_TOKEN(helper, ">|", "cstring_closed");
	REGIST_TOKEN(helper, "(", "bracket_opend");
	REGIST_TOKEN(helper, ")", "bracket_close");
	REGIST_TOKEN(helper, "//", "commetLine");

	// 
	REGIST_TOKEN(helper, " ", "space");
	REGIST_TOKEN(helper, "\t", "tab");
	REGIST_TOKEN(helper, "\n", "space");
	REGIST_TOKEN(helper, ":", "colon");
	REGIST_TOKEN(helper, ":=", "copy");
	REGIST_TOKEN(helper, ";", "context_closed");
	REGIST_TOKEN(helper, "*", "star");
	REGIST_TOKEN(helper, "<", "string_open");
	REGIST_TOKEN(helper, ">", "string_closed");
	REGIST_TOKEN(helper, "{", "node_open");
	REGIST_TOKEN(helper, "}", "node_closed");
	REGIST_TOKEN(helper, "[", "list_open");
	REGIST_TOKEN(helper, "]", "list_closed");
	REGIST_TOKEN(helper, "\\", "connectLine");

	// num
	REGIST_TOKEN(helper, "+", "add");
	REGIST_TOKEN(helper, "*", "mult");
	REGIST_TOKEN(helper, "=", "assign");
}

// 注册word
void regist_words(WordTypeHelper& helper) {
	REGIST_OPERATO_WORDS(helper, "call");
	REGIST_OPERATO_WORDS(helper, "time");
	REGIST_OPERATO_WORDS(helper, "add");
	REGIST_OPERATO_WORDS(helper, "echo");
	REGIST_OPERATO_WORDS(helper, "strcat");
	REGIST_OPERATO_WORDS(helper, "not");
	REGIST_OPERATO_WORDS(helper, "eq");
	REGIST_OPERATO_WORDS(helper, "g");
	REGIST_OPERATO_WORDS(helper, "l");
	REGIST_OPERATO_WORDS(helper, "typename");
	REGIST_OPERATO_WORDS(helper, "in");
	
	REGIST_CONTROLLER_WORDS(helper, "lambda");
	REGIST_CONTROLLER_WORDS(helper, "enclosed");
	REGIST_CONTROLLER_WORDS(helper, "return");
	REGIST_CONTROLLER_WORDS(helper, "break");

	REGIST_CONTROLLER_WORDS(helper, "abort");

	REGIST_CONTROLLER_WORDS(helper, "def");
	REGIST_CONTROLLER_WORDS(helper, "assign");
	REGIST_CONTROLLER_WORDS(helper, "cp");

	REGIST_CONTROLLER_WORDS(helper, "ignore");
	REGIST_CONTROLLER_WORDS(helper, "while");
	REGIST_CONTROLLER_WORDS(helper, "if");

	// num
	REGIST_OPERATO_WORDS(helper, "+");
	REGIST_OPERATO_WORDS(helper, "*");
	REGIST_CONTROLLER_WORDS(helper, "=");


	// 注册操作符优先级
	helper.regist_opera_2(Word(WordType::OPERATOR_WORD, "+"), 2);
	helper.regist_opera_2(Word(WordType::OPERATOR_WORD, "*"), 3);
	helper.regist_opera_2(Word(WordType::CONTROLLER, "="), 0);
}

// 注册语法铁路图
void regist_bnf(Parser& p) {
	Graphic_builder top_builder("top");
	BNFGraphic g_top = top_builder.rule()->
		nonterminal("block", "block")->
		terminal("EOS", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[EOS] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::EOS) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be EOS here\n";
			return false;
		}
	})->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_top);

	Graphic_builder block_builder("block");
	BNFGraphic g_block = block_builder.rule()->
		terminal("local_open", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[local_open] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::LOCAL_OPEN) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be '[' here\n";
			return false;
		}
	})->
		nonterminal("expr", "expr")->
		nonterminal("expr", "expr")->
		terminal("local_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[local_closed] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::LOCAL_CLOSED) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be ']' here\n";
			return false;
		}
	})->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_block);

	Graphic_builder expr_builder("expr");
	BNFGraphic g_expr = expr_builder.rule()->
		// block
		nonterminal("block", "block")->
		end()->
		gotoHead()->
		// list
		nonterminal("list", "list")->
		end()->
		gotoHead()->
		// control_expr
		nonterminal("control_expr", "control_expr")->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_expr);

	Graphic_builder control_expr_builder("control_expr");
	BNFGraphic g_control_expr =
		control_expr_builder.rule()->
		// if语句
		terminal("if", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[if] get word:" << w.serialize();
#endif
		if (w.getString() == "if") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be 'if' here\n";
			return false;
		}
	})->
		nonterminal("atomic_if", "atomic")->
		nonterminal("expr_if", "expr")->
		terminal("context_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[context_closed] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n";
			return false;
		}
	})->
		end()->
		gotoHead()->
		// while语句
		terminal("while", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[while] get word:" << w.serialize();
#endif
		if (w.getString() == "while") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be 'while' here\n";
			return false;
		}
	})->
		nonterminal("atomic_while", "atomic")->
		nonterminal("expr_while", "expr")->
		terminal("context_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[context_closed] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n";
				return false;
			}
		})->
		end()->
		gotoHead()->
		// break语句
		terminal("break", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[break] get word:" << w.serialize();
#endif
		if (w.getString() == "break") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be 'break' here\n";
			return false;
		}
	})->
		terminal("context_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[context_closed] get word:" << w.serialize();
#endif
			if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n";
				return false;
			}
		})->
		end()->
		gotoHead()->
		// return语句
		terminal("return", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[return] get word:" << w.serialize();
#endif
		if (w.getString() == "return") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be 'return' here\n";
			return false;
		}
	})->
		nonterminal("atomic_return", "atomic")->
		terminal("context_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[context_closed] get word:" << w.serialize();
#endif
			if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n";
				return false;
			}
		})->
		end()->
		gotoHead()->
		// abort语句
		terminal("abort", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[abort] get word:" << w.serialize();
#endif
			if (w.getString() == "abort") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be 'abort' here\n";
				return false;
			}
		})->
		nonterminal("atomic_abort", "atomic")->
		terminal("context_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[context_closed] get word:" << w.serialize();
#endif
			if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n";
				return false;
			}
		})->
		end()->
		gotoHead()->
		// time语句
		terminal("time", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[time] get word:" << w.serialize();
#endif
		if (w.getString() == "time") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be 'time' here\n";
			return false;
		}
	})->
		nonterminal("expr_time", "expr")->
		terminal("context_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[context_closed] get word:" << w.serialize();
#endif
			if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n";
				return false;
			}
		})->
		end()->
		gotoHead()->
		// ignore语句
		terminal("ignore", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[ignore] get word:" << w.serialize();
#endif
			if (w.getString() == "ignore") {
	#if CHECK_Parser
				cerr << ", True" << endl;
	#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be 'ignore' here\n";
				return false;
			}
		})->
		nonterminal("expr_ignore", "expr")->
		terminal("context_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[context_closed] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n";
				return false;
			}
		})->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_control_expr);

	// call_expr := block | atomic 
	Graphic_builder call_expr_builder("call_expr");
	BNFGraphic g_call_expr =
		call_expr_builder.rule()->
		nonterminal("block", "block")->
		end()->
		gotoHead()->
		nonterminal("atomic", "atomic")->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_call_expr);

	// var := 	<id_enabled> | "in" var obj_var ";"
	Graphic_builder var_builder("var");
	BNFGraphic g_var =
		var_builder.rule()->
		terminal("id_enabled", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[id_enabled] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::IDENTIFIER_ENABLED) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " is not an 'IDENTIFIER_ENABLED' word\n";
			return false;
		}
	})->
		end()->
		gotoHead()->
		// "in" var obj_var ";"
		terminal("in", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[in] get word:" << w.serialize();
#endif
			if (w.getString() == "in") {
	#if CHECK_Parser
				cerr << ", True" << endl;
	#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " might be 'in' here\n";
				return false;
			}
		})->
		nonterminal("var_in", "var")->
		nonterminal("obj_var_in", "obj_var")->
		terminal("context_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[context_closed] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " is not a ';' word\n";
			return false;
		}
	})->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_var);

	// obj_var := <number> | var
	Graphic_builder obj_var_builder("obj_var");
	BNFGraphic g_obj_var =
		obj_var_builder.rule()->
		terminal("number", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[number] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::NUMBER) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " is not an 'NUMBER' word\n";
			return false;
		}
	})->
		end()->
		gotoHead()->
		nonterminal("var", "var")->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_obj_var);

	// list := <operator> atomic { <seperator> atomic } ";" | def_expr
	Graphic_builder list_builder("list");
	BNFGraphic g_list =
		list_builder.rule()->
		terminal("operator", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[operator] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::OPERATOR_WORD) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " is not an 'OPERATOR_WORD' word\n";
			return false;
		}
	})->
		nonterminal("atomic", "atomic")->
		nonterminal("atomic", "atomic")->
		terminal("context_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[context_closed] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n";
			return false;
		}
	})->
		end()->
		// 定义和赋值语句
		gotoHead()->
		nonterminal("def_expr", "def_expr")->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_list);

	// atomic: = list | <number> | string | var | <IDENTIFIER_SPEC> | ("lambda" | "enclosed") atomic { atomic } block ";"
	Graphic_builder atomic_builder("atomic");
	BNFGraphic g_atomic =
		atomic_builder.rule()->
		nonterminal("list", "list")->
		end()->
		gotoHead()->
		terminal("number", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[number] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::NUMBER) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " is not a 'NUMBER' word\n";
			return false;
		}
	})->
		end()->
		gotoHead()->
		nonterminal("string", "string")->
		end()->
		gotoHead()->
		nonterminal("var", "var")->
		end()->
		gotoHead()->
		terminal("id_enabled", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[id_enabled] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::IDENTIFIER_ENABLED) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " is not an 'IDENTIFIER_ENABLED' word\n";
			return false;
		}
	})->
		end()->
		gotoHead()->
		terminal("IDENTIFIER_SPEC", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[IDENTIFIER_SPEC] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::IDENTIFIER_SPEC) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " is not an IDENTIFIER_SPEC word \n";
			return false;
		}
	})->
		end()->
		gotoHead()->
		// lambda语句
		terminal("lambda", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[lambda] get word:" << w.serialize();
#endif
			if (w.getString() == "lambda") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be 'lambda' here\n";
				return false;
			}
		})->
		terminal("id_enabled_lambda", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[id_enabled] get word:" << w.serialize();
#endif
			if (w.getType() == WordType::IDENTIFIER_ENABLED) {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
					return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " is not an 'IDENTIFIER_ENABLED' word\n";
				return false;
			}
		})->
		terminal("id_enabled_lambda")->
		nonterminal("block_lambda", "block")->
		terminal("context_closed_lambda", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[context_closed] get word:" << w.serialize();
#endif
			if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n";
				return false;
			}
		})->
		end()->
		gotoNode("lambda")->
		nonterminal("block_lambda", "block")->
		end()->
		gotoHead()->
		// 闭包语句
		terminal("enclosed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[enclosed] get word:" << w.serialize();
#endif
			if (w.getString() == "enclosed") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be 'enclosed' here\n";
				return false;
			}
		})->
		terminal("id_enabled_enclosed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[id_enabled] get word:" << w.serialize();
#endif
			if (w.getType() == WordType::IDENTIFIER_ENABLED) {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " is not an 'IDENTIFIER_ENABLED' word\n";
				return false;
			}
		})->
		terminal("id_enabled_enclosed")->
		nonterminal("block_enclosed", "block")->
		terminal("context_closed_enclosed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[context_closed] get word:" << w.serialize();
#endif
			if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n";
				return false;
			}
		})->
		end()->
		gotoNode("enclosed")->
		nonterminal("block_enclosed", "block")->
		end()->
		gotoHead()->
		// 单纯一个block
		nonterminal("block_only", "block")->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_atomic);

	Graphic_builder string_builder("string");
	BNFGraphic g_string =
		string_builder.rule()->
		terminal("string_open", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[local_open] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::STRING_OPEN) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be '<' here\n";
			return false;
		}
	})->
		terminal("string", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[local_open] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::STRING) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " is not an 'STRING' word\n";
			return false;
		}
	})->
		terminal("string", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[local_open] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::STRING) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " is not an 'STRING' word\n";
			return false;
		}
	})->
		terminal("string_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[string_closed] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::STRING_CLOSED) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be '>' here\n";
			return false;
		}
	})->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_string);

	// def_expr: = ("def" var proc | "assign" var proc | "cp" var proc ) ";"
	Graphic_builder def_expr_builder("def_expr");
	BNFGraphic g_def_expr =
		def_expr_builder.rule()->
		// def语句
		terminal("def", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[def] get word:" << w.serialize();
#endif
		if (w.getString() == "def") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be 'def' here\n";
			return false;
		}
	})->
		nonterminal("var_def", "var")->
		nonterminal("atomic_def", "atomic")->
		terminal("context_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[context_closed] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n";
			return false;
		}
	})->
		end()->
		gotoHead()->
		// assign语句
		terminal("assign", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[assign] get word:" << w.serialize();
#endif
		if (w.getString() == "assign") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be 'assign' here\n";
			return false;
		}
	})->
		nonterminal("var_assign", "var")->
		nonterminal("atomic_assign", "atomic")->
		terminal("context_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[context_closed] get word:" << w.serialize();
#endif
			if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n";
				return false;
			}
		})->
		end()->
		gotoHead()->
		// cp语句
		terminal("cp", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[cp] get word:" << w.serialize();
#endif
		if (w.getString() == "cp") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be 'cp' here\n";
			return false;
		}
	})->
		nonterminal("var_cp", "var")->
		nonterminal("atomic_cp", "atomic")->
		terminal("context_closed", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[context_closed] get word:" << w.serialize();
#endif
			if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n";
				return false;
			}
		})->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_def_expr);
}

// 写入文件 []][
void write_to_file() {
	// string str("abcdaa");
	op_code_bit_set bitset(OPCODE::ADD, 0x11223344);	//str.c_str(), str.size());

	// 写入文件
	fstream fs("out.tr", ios_base::out);
	// 写入m次opcode
	int m = 3;
	for (int i = 0; i < m; ++i) {
		// bitset.display();
		fs.write(*bitset.to_bit_set(), bitset.write_size());
	}
	fs.close();

	// 从文件中读取
	fstream ofs("out.tr", ios_base::in | ios::binary);
	// 读取n个opcode
	int n = 3;
	for (int i = 0; i < n; ++i) {
		// 读取字符串位
		bool is_string;
		is_string = ofs.get();
		cout << is_string << endl;

		// 读取opcode
		int op;
		ofs.read((char*)&op, 4);
		cout << hex << op << endl;

		// 读取要push的data
		if (is_string) {
			string data;
			for (char c; (c = ofs.get()) != '\0';) {
				data += c;
			}
			cout << data << endl;
		}
		else {
			char temp[sizeof(int) + 1];
			int data;
			ofs.read(temp, sizeof(int) + 1);
			data = *(int*)&temp;
			cout << hex << data << endl;
		}
		cout << endl;
	}
	ofs.close();
}

// 前缀转中缀 []][
void test_app() {

	// 准备工作
	Context_helper ctx_helper;
	regist_keywords_contents(ctx_helper);

	Token_helper token_helper;
	regist_token(token_helper);

	WordTypeHelper word_type_helper;
	regist_words(word_type_helper);

	// 初始化lexer
#if MODE_CIL
	Lexer lex(new CLIInput("luo ->"));
#else
	Lexer lex(new FileInput("in.tr"));
#endif

	// 初始化适配器
	Mid_Expr_Adaptor adptor;
#if MODE_CIL
	while (1)
#endif
	{
		// 词法分析
		lex.init();
		lex.fillList(token_helper, word_type_helper);
		std::vector<Word>& words = lex.get_words_list();

		for (auto w : words) {
			cout << w.serialize() << ends;
		}

		cout << endl << "转为前缀表达式: " << endl;

		auto adapts = adptor.adapt_code(words, word_type_helper);
		for (auto w : adapts) {
			cout << w.getString() << ends;
		}
		cout << endl;
	}
}

// 测试命令行
void test_input_cli() {

	// 准备工作
	Context_helper ctx_helper;
	regist_keywords_contents(ctx_helper);

	Token_helper token_helper;
	regist_token(token_helper);

	WordTypeHelper word_type_helper;
	regist_words(word_type_helper);

	// 初始化scanner
#if MODE_CIL
	Lexer lex(new CLIInput("luo ->"));
#else
	Lexer lex(new FileInput("in.tr"));
#endif

	// 注册语法规则
	Parser p(&lex);
	regist_bnf(p);

	// 编译器
	S_Expr_Compiler compiler;
	Compile_result cresult;

	// 虚拟机
	vsVirtualMachine vm(0);

#if MODE_CIL
	while (1)
#endif
	{
		p.clear_word_vec();

		// 词法分析
		lex.init();
		lex.fillList(token_helper, word_type_helper);

#if CHECK_Lexer
		do {
			cerr << "[" << lex.peek().serialize() << "], " << endl;
		} while (lex.next());
#endif
		try {
			// 语法分析
			int b = p.parse("top");
			cerr << "Parse " << (b ? "success!" : "failed!") << endl;
			if (!b) {
				cout << "Parse ERROR =\n" << p.getErrors() << endl;
			}
			else
			{
				// 生成目标代码

				compiler.generate_code(p.get_word_vector_ref(), cresult, ctx_helper);	// 会抛出异常
				// compiler.generate_code(lex.get_words_list(), cresult, ctx_helper);	// 会抛出异常
				cout << "Code Generated finished!" << endl << endl;

				// 执行代码
				vm.init(cresult, 1);
				vm.run();
#if MODE_CIL
				if (vm.get_eval_ret_value() == -1)break;
#endif
			}
		}
		catch (Context_found_error e) {
			cerr << e.what() << endl;
		}
		catch (stack_overflow_exception e) {
			cerr << e.what() << endl;
		}
		catch (undefined_exception e) {
			cerr << e.what() << endl;
		}
		// 清除结果
		compiler.init();
		cresult.init();
	}
}

// 
int main(int argc, char* argv[]) {
//	test_app();
	test_input_cli();
	return 0;
};