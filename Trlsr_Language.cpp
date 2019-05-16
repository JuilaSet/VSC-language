// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"

using namespace std;

////////////////
// 注册语言层 //
////////////////

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

	helper.regist_context(Word(WordType::CONTROLLER, "delete").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {
				compiler->in_def();
				return _command_set{};
			},
			{
				Context([&](ContextStk& cstk, _command_set& commandVec, Word& w,  auto* compiler) {
					compiler->out_def();
					compiler->dont_gene_callblk();
					return _command_set{};
				}, Context_Type::NORMAL, "def_op1"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->enable_gene_callblk();
					return _command_set{
						COMMAND(NEW_DEL)
					};
				}, Context_Type::END, "def_end")
			})
	);

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

	helper.regist_context(Word(WordType::CONTROLLER, "extern").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {
				compiler->in_def(); 
				return _command_set{}; 
			},
			{
				// 转为数字
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_*
					// 获取分配的下标
					compiler->insert_local(w, WordType::IDENTIFIER);	// 告知Compiler声明过了第一个参数
					return _command_set{
						COMMAND(EXTERN)
					};
				}, Context_Type::ALWAYS, "extern_op*"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->out_def();
					return _command_set{ };
				}, Context_Type::END, "extern_end")
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
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					int last_index = helper.pop_command_index();
					int addr = _vec.size();
					_vec[last_index] = CommandHelper::getPushOpera(data_ptr(new AddrData(addr + 2)));
					helper.push_command_index(_vec.size());
					return _command_set{
						COMMAND(ERROR),		// 待确定
						COMMAND(JMP) 
					};
				}, Context_Type::NORMAL, "if_p2"),
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					int last_index = helper.pop_command_index();
					int addr = _vec.size(); // 获取当前地址
					_vec[last_index] = CommandHelper::getPushOpera(data_ptr(new AddrData(addr))); // 回填
					return _command_set{ };
				}, Context_Type::NORMAL, "if_p3"),
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					return _command_set{ };
				}, Context_Type::END, "if_end")
			})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "nop").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {
#if CHECK_Compiler
			cout << "COMMAND:: nop" << endl;
#endif
			return _command_set{
					COMMAND(NOP)
			};
		}, { })	// 只能有一个上下文
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

	helper.regist_context(Word(WordType::IDENTIFIER_SPEC, "$self").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {
#if CHECK_Compiler
		cout << "COMMAND:: $self" << endl;
#endif
			return _command_set{
				COMMAND(SELF)
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

	helper.regist_context(Word(WordType::OPERATOR_WORD, "get").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {
		compiler->save_def();
		return _command_set{};
	},
			{
				Context([&](ContextStk& cstk, _command_set& commandVec, Word& w,  auto* compiler) {
					return _command_set{};
				}, Context_Type::NORMAL, "in_op1"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{
						COMMAND(IN)
					};
				}, Context_Type::NORMAL, "in_op2"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
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
						return _command_set{
							COMMAND(ADD)
						};
				}, Context_Type::ALWAYS, "add_op2"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_end
					return _command_set{ };
				}, Context_Type::END, "add_end")
			})
	);

	helper.regist_context(Word(WordType::OPERATOR_WORD, "sub").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {	// op_0
				return _command_set{ };
			},
			{
				// 转为数字
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_1
					return _command_set{ };
				}, Context_Type::NORMAL, "sub_op1"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_2
						return _command_set{
							COMMAND(SUB)
						};
				}, Context_Type::ALWAYS, "sub_op2"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_end
					return _command_set{ };
				}, Context_Type::END, "sub_end")
			})
	);

	helper.regist_context(Word(WordType::OPERATOR_WORD, "not").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {	// op_0
		return _command_set{ };
	},
			{
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_1
					return _command_set{ };
				}, Context_Type::NORMAL, "not_op1"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_end
					return _command_set{
						COMMAND(NOT)
					};
				}, Context_Type::END, "not_end")
			})
	);

	helper.regist_context(Word(WordType::OPERATOR_WORD, "eq").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {	// op_0
				return _command_set{ };
			},
			{
				// 转为数字
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_1
					return _command_set{ };
				}, Context_Type::NORMAL, "eq_op1"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_2
						return _command_set{ };
				}, Context_Type::NORMAL, "eq_op2"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_end
					return _command_set{
						COMMAND(EQ)
					};
				}, Context_Type::END, "eq_end")
			})
	);

	helper.regist_context(Word(WordType::OPERATOR_WORD, "g").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {	// op_0
		return _command_set{ };
	},
			{
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_1
					return _command_set{ };
				}, Context_Type::NORMAL, "g_op1"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_2
						return _command_set{ };
				}, Context_Type::NORMAL, "g_op2"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_end
					return _command_set{
						COMMAND(G)
					};
				}, Context_Type::END, "g_end")
			})
	);

	helper.regist_context(Word(WordType::OPERATOR_WORD, "l").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {	// op_0
				return _command_set{ };
			},
			{
				// 转为数字
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_1
					return _command_set{ };
				}, Context_Type::NORMAL, "l_op1"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_2
						return _command_set{ };
				}, Context_Type::NORMAL, "l_op2"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_end
					return _command_set{
						COMMAND(L)
					};
				}, Context_Type::END, "l_end")
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

	helper.regist_context(Word(WordType::OPERATOR_WORD, "vec").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w, auto* compiler) {
				compiler->dont_gene_callblk();
				return _command_set{
						CommandHelper::getPushOpera(data_ptr(new vsVector)) 
				};
			},
			{
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{
						COMMAND(VEC_PUSH)
					};
				}, Context_Type::ALWAYS, "echo_op*"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->enable_gene_callblk();
					return _command_set{ };
				}, Context_Type::END, "echo_end")
			})
	);

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
	REGIST_TOKEN(helper, "\n", "eol");
	REGIST_TOKEN(helper, "<", "string_open");
	REGIST_TOKEN(helper, ">", "string_closed");
	REGIST_TOKEN(helper, "[", "list_open");
	REGIST_TOKEN(helper, "]", "list_closed");
	REGIST_TOKEN(helper, "\\", "connectLine");
	REGIST_TOKEN(helper, ";", "context_closed");

	// num
//	REGIST_TOKEN(helper, "+", "add");
//	REGIST_TOKEN(helper, "*", "mult");
//	REGIST_TOKEN(helper, "=", "assign");
}

// 注册word
void regist_words(WordTypeHelper& helper) {
	REGIST_OPERATO_WORDS(helper, "call");
	REGIST_OPERATO_WORDS(helper, "time");
	REGIST_OPERATO_WORDS(helper, "add");
	REGIST_OPERATO_WORDS(helper, "sub");
	REGIST_OPERATO_WORDS(helper, "echo");
	REGIST_OPERATO_WORDS(helper, "not");
	REGIST_OPERATO_WORDS(helper, "eq");
	REGIST_OPERATO_WORDS(helper, "g");
	REGIST_OPERATO_WORDS(helper, "l");
	REGIST_OPERATO_WORDS(helper, "typename");
	REGIST_OPERATO_WORDS(helper, "in");
	REGIST_OPERATO_WORDS(helper, "get");
	REGIST_OPERATO_WORDS(helper, "vec"); 
	
	REGIST_CONTROLLER_WORDS(helper, "lambda");
	REGIST_CONTROLLER_WORDS(helper, "enclosed");
	REGIST_CONTROLLER_WORDS(helper, "return");
	REGIST_CONTROLLER_WORDS(helper, "break");

	REGIST_CONTROLLER_WORDS(helper, "abort");
	REGIST_CONTROLLER_WORDS(helper, "extern");

	REGIST_CONTROLLER_WORDS(helper, "def");
	REGIST_CONTROLLER_WORDS(helper, "delete");
	REGIST_CONTROLLER_WORDS(helper, "assign");
	REGIST_CONTROLLER_WORDS(helper, "cp");

	REGIST_CONTROLLER_WORDS(helper, "ignore");
	REGIST_CONTROLLER_WORDS(helper, "while");
	REGIST_CONTROLLER_WORDS(helper, "if");
	REGIST_CONTROLLER_WORDS(helper, "nop");

	// num
//	REGIST_OPERATO_WORDS(helper, "+");
//	REGIST_OPERATO_WORDS(helper, "*");
//	REGIST_CONTROLLER_WORDS(helper, "=");


	// 注册操作符优先级
//	helper.regist_opera_2(Word(WordType::OPERATOR_WORD, "+"), 2);
//	helper.regist_opera_2(Word(WordType::OPERATOR_WORD, "*"), 3);
//	helper.regist_opera_2(Word(WordType::CONTROLLER, "="), 0);
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
		// nop语句
		terminal("nop", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[nop] get word:" << w.serialize();
#endif
			if (w.getString() == "nop") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be 'nop' here\n";
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
		// extern语句
		terminal("extern", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[break] get word:" << w.serialize();
#endif
		if (w.getString() == "extern") {
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
		// if语句
		gotoHead()->
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
		nonterminal("expr_if_1", "expr")->
		nonterminal("expr_if_2", "expr")->
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

	// def_expr: = ( "type_assert" string | "def" var proc | "assign" var proc | "cp" var proc ) ";"
	Graphic_builder def_expr_builder("def_expr");
	BNFGraphic g_def_expr =
		def_expr_builder.rule()->
		// delete语句
		terminal("delete", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[delete] get word:" << w.serialize();
#endif
			if (w.getString() == "delete") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be 'delete' here\n";
				return false;
			}
		})->
		nonterminal("var_def", "var")->
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

////////////////
// 注册第一层 //
////////////////

// 注册符号给第一层输入
void regist_token_level1(Token_helper& helper) {
	// 
	REGIST_TOKEN(helper, "/*", "commet_open");
	REGIST_TOKEN(helper, "*/", "commet_close");
	REGIST_TOKEN(helper, "//", "commetLine");
	// 分隔符
	REGIST_TOKEN(helper, " ", "space");
	REGIST_TOKEN(helper, "\t", "tab");
	REGIST_TOKEN(helper, "\n", "space");
	// 结点设置
	REGIST_TOKEN(helper, "*", "star");
	REGIST_TOKEN(helper, ":", "colon");
	REGIST_TOKEN(helper, "{", "node_open");
	REGIST_TOKEN(helper, "}", "node_closed");
	REGIST_TOKEN(helper, ";", "context_closed");
	REGIST_TOKEN(helper, "<", "string_open");
	REGIST_TOKEN(helper, ">", "string_closed");
	// 结点连接
	REGIST_TOKEN(helper, "->", "link");
	// 开启会话
	REGIST_TOKEN(helper, "#", "session");
}

// 注册word, 让字符保留
void regist_words_level1(WordTypeHelper& helper) {
	// attribute: = "ins" | "out" | "calc" | "timeout"
	REGIST_OPERATO_WORDS(helper, "ins");
	REGIST_OPERATO_WORDS(helper, "out");
	REGIST_OPERATO_WORDS(helper, "calc");
	REGIST_OPERATO_WORDS(helper, "timeout");

	// 
	REGIST_CONTROLLER_WORDS(helper, "*");
	REGIST_CONTROLLER_WORDS(helper, ":");
	REGIST_CONTROLLER_WORDS(helper, "{");
	REGIST_CONTROLLER_WORDS(helper, "}");
	REGIST_CONTROLLER_WORDS(helper, "->");
	REGIST_CONTROLLER_WORDS(helper, "#");
}

// 注册语法
void regist_bnf_level1(Parser& p) {

	// 顶层: = { 结点声明语句 } { 结点连接语句 } { 开启会话 } EOS
	Graphic_builder top_builder("top");
	BNFGraphic g_top = top_builder.rule()->
		nonterminal("node_def_expr", "node_def_expr")->
		nonterminal("node_def_expr", "node_def_expr")->
		nonterminal("node_link_expr", "node_link_expr")->
		nonterminal("node_link_expr", "node_link_expr")->
		nonterminal("session_expr", "session_expr")->
		nonterminal("session_expr", "session_expr")->
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

	// 结点声明语句 := "*" id_enabled "{" attr_def { attr_def } "}"
	Graphic_builder node_def_expr_builder("node_def_expr");
	BNFGraphic g_node_def_expr = node_def_expr_builder.rule()->
		terminal("*", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[*] get word:" << w.serialize();
#endif
			if (w.getString() == "*") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be * here\n";
				return false;
			}
		})->
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
		terminal("{", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[{] get word:" << w.serialize();
#endif
			if (w.getString() == "{") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be { here\n";
				return false;
			}
		})->
		nonterminal("attr_def", "attr_def")->
		nonterminal("attr_def", "attr_def")->
		terminal("}", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[}] get word:" << w.serialize();
#endif
			if (w.getString() == "}") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be } here\n";
				return false;
			}
		})->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_node_def_expr);

	// attr_def := "ins" id_enabled { id_enabled } ";" | "out" id_enabled ";" | calc string ";" | timeout number ";"
	Graphic_builder attr_def_builder("attr_def");
	BNFGraphic g_attr_def = attr_def_builder.rule()->
		terminal("ins", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[:ins] get word:" << w.serialize();
#endif
			if (w.getString() == "ins") {
	#if CHECK_Parser
				cerr << ", True" << endl;
	#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ins here\n";
				return false;
			}
		})->
		terminal(":_ins", [](Word w, std::string& err, int lex_point) {
	#if CHECK_Parser
			cerr << "[:] get word:" << w.serialize();
	#endif
			if (w.getString() == ":") {
	#if CHECK_Parser
				cerr << ", True" << endl;
	#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be : here\n";
				return false;
			}
		})->
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
		terminal(";", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[:] get word:" << w.serialize();
#endif
			if (w.getString() == ";") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ; here\n";
				return false;
			}
		})->
		end()->
		gotoHead()->
		terminal("out", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[out] get word:" << w.serialize();
#endif
			if (w.getString() == "out") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be out here\n";
				return false;
			}
		})->
		terminal(":_out", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[:] get word:" << w.serialize();
#endif
			if (w.getString() == ":") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be : here\n";
				return false;
			}
		})->
		terminal("id_enabled_out", [&](Word w, std::string& err, int lex_point) {
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
		terminal(";", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[:] get word:" << w.serialize();
#endif
			if (w.getString() == ";") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ; here\n";
				return false;
			}
		})->
		end()->
		gotoHead()->
		terminal("calc", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[calc] get word:" << w.serialize();
#endif
			if (w.getString() == "calc") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ins here\n";
				return false;
			}
		})->
		terminal(":_calc", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[:] get word:" << w.serialize();
#endif
			if (w.getString() == ":") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be : here\n";
				return false;
			}
		})->
		nonterminal("string", "string")->
		terminal(";", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[:] get word:" << w.serialize();
#endif
			if (w.getString() == ";") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
		}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ; here\n";
				return false;
			}
		})->
		end()->
		gotoHead()->
		terminal("timeout", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[timeout] get word:" << w.serialize();
#endif
			if (w.getString() == "timeout") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be timeout here\n";
				return false;
			}
		})->
		terminal(":timeout", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[:] get word:" << w.serialize();
#endif
			if (w.getString() == ":") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be : here\n";
				return false;
			}
		})->
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
		terminal(";", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[:] get word:" << w.serialize();
#endif
			if (w.getString() == ";") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ; here\n";
				return false;
			}
		})->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_attr_def);

	// 结点连接语句 : = id_enabled "->" id_enabled { "->" id_enabled } ";"
	Graphic_builder node_link_expr_builder("node_link_expr");
	BNFGraphic g_node_link_expr = node_link_expr_builder.rule()->
		terminal("id_enabled_1", [&](Word w, std::string& err, int lex_point) {
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
		terminal(";", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[:] get word:" << w.serialize();
#endif
		if (w.getString() == ";") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be ; here\n";
			return false;
		}
	})->
		end()->
		gotoNode("id_enabled_1")->
		terminal("->", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[->] get word:" << w.serialize();
#endif
			if (w.getString() == "->") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be -> here\n";
				return false;
			}
		})->
		terminal("id_enabled_2", [&](Word w, std::string& err, int lex_point) {
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
		terminal(";", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[:] get word:" << w.serialize();
#endif
		if (w.getString() == ";") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
	}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be ; here\n";
			return false;
		}
		})->
		end()->
		gotoNode("id_enabled_2")->
		terminal("->", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[->] get word:" << w.serialize();
#endif
			if (w.getString() == "->") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be -> here\n";
				return false;
			}
		})->
		getGraphic();
		p.addBNFRuleGraphic(g_node_link_expr);

	// 开启会话: = "#"  id_enabled
	Graphic_builder session_expr_builder("session_expr");
	BNFGraphic g_session_expr = session_expr_builder.rule()->
		terminal("#", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[#] get word:" << w.serialize();
#endif
			if (w.getString() == "#") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be # here\n";
				return false;
			}
		})->
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
		terminal(";", [](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[:] get word:" << w.serialize();
#endif
			if (w.getString() == ";") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be ; here\n";
				return false;
			}
		})->
		end()->
		getGraphic();
		p.addBNFRuleGraphic(g_session_expr);

	// string : = "<" string ">"
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
}

// 获取原型图
std::shared_ptr<Node_Compiler> getGraphicMessage() throw(ParserFailed) {
	// 准备工作
	Token_helper level1_token_helper;
	regist_token_level1(level1_token_helper);

	WordTypeHelper level1_word_type_helper;
	regist_words_level1(level1_word_type_helper);

	// 初始化scanner
#if MODE_CIL
	Lexer lex(new CLIInput("luo ->"));
#else
	Lexer lex(new FileInput("in.tr"));
#endif

	// 注册语法规则
	Parser p(&lex);
	regist_bnf_level1(p);

#if MODE_CIL
	while (1)
#endif
	{
		p.clear_word_vec();

		// 词法分析
		lex.init();
		lex.fillList(level1_token_helper, level1_word_type_helper);
		// 语法分析
		int b = p.parse("top");
		cerr << (b ? "" : "Graphic Parse failed!") << endl;
		if (!b) {
			cout << "Parse ERROR =\n" << p.getErrors() << endl;
			throw ParserFailed("Parse failed");
		}
		else {
			auto& vec = p.get_word_vector_ref();
			std::shared_ptr<Node_Compiler> nodecmp(new Node_Compiler(vec));
			nodecmp->top();
			return nodecmp;
		}
	}
}

// 根据原型图来实例化一个图
vsThread::taskGraphic_ptr<std::string, data_ptr>
buildTasksGraphic(vsTool::graphic_ptr<std::shared_ptr<LeafNode>>& proto_graphic, vsVirtualMachine& vm) {
#if CHECK_Tool
	proto_graphic->show();
#endif

	// 初始化计算图
	vsThread::taskGraphic_ptr<std::string, data_ptr> _taskGraphic(new vsThread::TaskGraphic<std::string, data_ptr>);
	
	// 初始化结点
	proto_graphic->for_each_unordered([&vm, &_taskGraphic](auto graphic, auto node) {
		auto pnode = node->get_data();
		vsTool::id_t nodeid = pnode->getId();
		// 接收信号
		data_ptr ins = pnode->getData("ins");
		std::vector<std::string> data_list;
		if (ins->getType() != DataType::NON) {
			// 获得数组容器
			std::shared_ptr<vsLinearContainer> container = vsVector::cast_vsVector_ptr(ins)->getVec();

			// 转化为数组对象
			do {
				// 将对象放入vec
				data_ptr data = container->cur_data();
				data_list.push_back(data->toString());
#if CHECK_Compiler_Node_res
				std::cout << __LINE__ << "\t添加ins: " << data->toString() << std::endl;
#endif
			} 
			while(container->next());
		}

#if CHECK_Compiler_Node_res
		std::cout << __LINE__ << "\t添加eval: " << nodeid << std::endl;
#endif

		auto _eval = vm.make_eval(nodeid);
		// 设置eval返回信号
		data_ptr out = pnode->getData("out");
		if (out->getType() != DataType::NON) {
			// 获得数组容器
			std::shared_ptr<vsLinearContainer> container = vsVector::cast_vsVector_ptr(out)->getVec();
			data_ptr res_str = container->cur_data();
			_eval->set_sign_str(res_str->toString());
		}
#if CHECK_Compiler_Node_res
		std::cout << __LINE__ << " id : " << nodeid << " 返回信号: " << ret << std::endl;
#endif
		_taskGraphic->build_node([=](auto& _out, auto& datas, auto& keys) {
					// 设置外部_data
					for(auto& pair : datas){
						_eval->_add_extern_data(pair.first, pair.second);
					}

					// 主线程进入入口点, 相当于直接 <call_blk _enter_point>
					_eval->load_block(1, 0);

					// 执行
					_eval->eval();
			
					// 设置返回值
					_out = _eval->_get_ret_data();

					// 返回信号
					return _eval->get_sign_str();
				}, 
				data_list,
				nodeid, 
				pnode->getData("timeout")->toNumber()
			);
	});

	// 初始化图结构, 遍历每条边
	vsTool::id_t findex;
	_taskGraphic->head();
	proto_graphic->for_each_dfs_norepeat([&_taskGraphic, &findex](auto g, auto node_id) {
			// 获取结点
			auto node = g->get_node_ptr(node_id)->get_data();
			findex = node->getId();
			_taskGraphic->gotoNode(findex);
			return true;
		},  
		[&_taskGraphic, &findex](auto g, auto node_id) {
			// 获取结点
			auto node = g->get_node_ptr(node_id)->get_data();
			auto nindex = node->getId();
			_taskGraphic->linkToNode(nindex);
			_taskGraphic->gotoNode(findex);
			return true;
		});
	_taskGraphic->end();

	// 设置结果
	return _taskGraphic;
}

// 编辑图结点的代码
void addProcFromProtoNode(vsTool::graphic_ptr<std::shared_ptr<LeafNode>>& proto_graphic, vsVirtualMachine& vm, vsFrontend front) {
	// 初始化结点
	proto_graphic->for_each_unordered([&vm, &front](auto graphic, auto node) {
		auto pnode = node->get_data();
		vsTool::id_t nodeid = pnode->getId();
		std::string calc = pnode->getData("calc")->toString();

#if CHECK_Compiler_Node_res
		std::cout << __LINE__ << "添加proc: " << nodeid << std::endl;
#endif

		vm.add_process(front.getResultByString(calc), nodeid); // 编译结果, 代码段id = node的id
	});
}

// 测试编译前端
void test_vsFrontend() {
	try {
		// 编译前端
		vsFrontend front(regist_bnf, regist_keywords_contents, regist_token, regist_words);

		// 虚拟机
		vsVirtualMachine vm(0);

		// 搭建图
		auto nodecmp = getGraphicMessage();

		// 获取原型图
		auto graphic_proto = nodecmp->getGraphic();

		// 根据原型图创建计算图
		auto graphic = buildTasksGraphic(graphic_proto, vm);

		// 设置虚拟机
		vm.set_graphic(graphic);
		addProcFromProtoNode(graphic_proto, vm, front);

		// 执行会话
		vm.run(nodecmp->ret_sess(), "x");
	}
	catch (Context_error& e) {
		cerr << e.what() << endl;
	}
	catch (ParserFailed& e) {
		cerr << e.what() << endl;
	}
	catch (undefined_exception& e) {
		std::cerr << e.what() << std::endl;
	}
	catch (stack_overflow_exception& e) {
		cerr << e.what() << endl;
	}
	catch (run_time_exception& e) {
		cerr << e.what() << endl;
	}
}

// 
int main(int argc, char* argv[]) {
	test_vsFrontend();
	return 0;
};