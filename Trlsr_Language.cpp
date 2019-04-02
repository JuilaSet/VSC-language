﻿// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <fstream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <regex>

using namespace std;

// 注册关键字到上下文的关联
void regist_keywords_contents(Context_helper& helper) {
	const std::string PARA_WORD = "paras_";

	// CONTROLLER
	helper.regist_context(Word(WordType::CONTROLLER, "call").serialize(),
		helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
			compiler->paras_begin();
			return _command_set{ };
		},
		{
			EMPTY_CONTEXT,
			Context([=](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				std::string n = std::to_string(compiler->paras()++);
				return _command_set{
						CommandHelper::getDefOpera(PARA_WORD + n)
				};
			}, Context_Type::ALWAYS), 
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				compiler->paras_end();
				return _command_set{
						Command(OPERATOR::CALL)
				};
			}, Context_Type::END)
		})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "time").serialize(),
		helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
		helper.push_command_index(_vec.size());
			return _command_set{
					Command(OPERATOR::TIME_BEGIN)
			};
		},
		{
			Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
				return _command_set{ };
			}, Context_Type::NORMAL, "time_block"),
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				return _command_set{
						Command(OPERATOR::TIME_END)
				};
			}, Context_Type::END, "time_end")
		})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "prcd").serialize(),
		helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
			helper.push_command_index(_vec.size());
			return _command_set{
					Command(OPERATOR::ERROR),	// 待确定
					Command(OPERATOR::JMP)
			};
		},
		{
			Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
				return _command_set{ };
			}, Context_Type::NORMAL, "prcd_op1"),
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				int last_index = helper.pop_command_index();
				int addr = _vec.size();
				_vec[last_index] = CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, addr + 1));
				return _command_set{
						Command(OPERATOR::RET),
						CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, last_index + 2))	// push过程的首地址
				};
			}, Context_Type::END, "prcd_end")
		})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "tuple").serialize(),
		helper.build_context(
		[](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {	// op_0
			return _command_set{ CommandHelper::getPushOpera(Data()) };	// 存入void
		},
		{
			Context([](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {	// op_always
				return _command_set{ };
			}, Context_Type::ALWAYS, "tuple_op1"),
			Context([](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {	// op_end
				return _command_set{ };
			}, Context_Type::END, "tuple_end")
		})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "for").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				return _command_set{};
			},
			{
				// tuple位置
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					helper.push_command_index(_vec.size());
					helper.push_command_index(_vec.size() + 1);
					compiler->in_def();
					return _command_set{
						Command(OPERATOR::ISNON),
						Command(OPERATOR::ERROR),		// 待确定
						Command(OPERATOR::JMP_TRUE)
					};
				}, Context_Type::NORMAL, "for_tuple"),
				// 标识符
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					compiler->out_def();
					compiler->insert_local(w, WordType::IDENTIFIER);	// 告知parser声明过了第一个参数
					return _command_set{
						Command(OPERATOR::REVERSE_TOP),
						Command(OPERATOR::DEF),
						Command(OPERATOR::POP)
					};
				}, Context_Type::NORMAL, "for_op2"),
				// 结束
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					int last_index = helper.pop_command_index();
					int head_index = helper.pop_command_index();
					int addr = _vec.size();
					_vec[last_index] = CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, addr + 2));
					return _command_set{
						CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, head_index)),
						Command(OPERATOR::JMP),
						Command(OPERATOR::POP)	// pop void值
					};
				}, Context_Type::END, "for_end")
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
					// 获取分配的下标
					int index = compiler->insert_local(w, WordType::IDENTIFIER);	// 告知Compiler声明过了第一个参数
					commandVec.pop_back();
					commandVec.push_back(CommandHelper::getPushOpera(Data(DataType::NUMBER, index)));
					return _command_set{};
				}, Context_Type::NORMAL, "def_op1"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{};
				}, Context_Type::NORMAL, "def_op2"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{
						// Command(OPERATOR::DEF)
						Command(OPERATOR::NEW_DEF)
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
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					compiler->out_def();
					return _command_set{};
				}, Context_Type::NORMAL, "assign_op1"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{};
				}, Context_Type::NORMAL, "assign_op2"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{
						// Command(OPERATOR::ASSIGN)
						Command(OPERATOR::NEW_ASSIGN)
					};
				}, Context_Type::END, "assign_end")
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
							Command(OPERATOR::CAST_NUMBER)
						};
					}
					else {
						return _command_set{ };
					}
				}, Context_Type::NORMAL, "abort_op1"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{
						Command(OPERATOR::ABORT)
					};
				}, Context_Type::END)
			})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "ignore").serialize(),
		helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
			helper.push_command_index(_vec.size());
			return _command_set{
					Command(OPERATOR::ERROR),	// 待确定
					Command(OPERATOR::JMP)
			};
		},
		{
			EMPTY_CONTEXT,
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
				int last_index = helper.pop_command_index();
				int addr = _vec.size();
				_vec[last_index] = CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, addr));
				return _command_set{
						Command(OPERATOR::NOP)
				};
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
						Command(OPERATOR::TEST),
						Command(OPERATOR::ERROR),		// 待确定
						Command(OPERATOR::JMP_FALSE)
					};
				}, Context_Type::NORMAL, "if_p1"),
				EMPTY_CONTEXT,
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					int last_index = helper.pop_command_index();
					int addr = _vec.size();
					_vec[last_index] = CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, addr));
					return _command_set{
						Command(OPERATOR::NOP)
					};
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
						Command(OPERATOR::TEST),
						Command(OPERATOR::ERROR),		// 待确定
						Command(OPERATOR::JMP_FALSE)
					};
				}),
				EMPTY_CONTEXT,
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					int last_index = helper.pop_command_index();
					int head_index = helper.pop_command_index();
					int addr = _vec.size();
					_vec[last_index] = CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, addr + 2));
					return _command_set{
						CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, head_index)),
						Command(OPERATOR::JMP),
						Command(OPERATOR::NOP)
					};
				}, Context_Type::END)
			})
	);

	helper.regist_context(Word(WordType::CONTROLLER, "rept").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {	return _command_set{}; },
			{
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					helper.push_command_index(_vec.size() + 1);
					return _command_set {
						Command(OPERATOR::COUNT),
						Command(OPERATOR::NOP)
					};
				}, Context_Type::NORMAL, "rept_op1"),
				// block
				Context([&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{ };
				}, Context_Type::NORMAL, "rept_block"),
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {
					int addr = helper.pop_command_index();
					return _command_set{
						CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, addr)),
						Command(OPERATOR::REPT)};
				}, Context_Type::END, "rept_end")
			})
	);
	
	// IDENTIFIER_SPEC
	
	helper.regist_context(Word(WordType::IDENTIFIER_SPEC, "$C").serialize(),
		helper.build_context(
		[](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_0
#if CHECK_Compiler
		cout << "COMMAND:::: $C" << endl;
#endif
			return _command_set{ Command(OPERATOR::ECX) };
		},{ })	// 只能有一个上下文
	);

	helper.regist_context(Word(WordType::IDENTIFIER_SPEC, "$null").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_0
#if CHECK_Compiler
		cout << "COMMAND:::: $null" << endl;
#endif
		return _command_set{ Command(OPERATOR::NUL) };
	}, { })	// 只能有一个上下文
	);

	// OPERATOR_WORD

	helper.regist_context(Word(WordType::OPERATOR_WORD, "strcat").serialize(),
		helper.build_context(
			[](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {	// op_0
				return _command_set{ };
			},
			{
				// 转为字符串
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_1
					if (w.getType() != WordType::STRING) {
						return _command_set{
							Command(OPERATOR::CAST_STRING)
						};
					}
					else {
						return _command_set{ };
					}
				}),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_2
					if (w.getType() != WordType::STRING) {
						return _command_set{
							Command(OPERATOR::CAST_STRING)
						};
					}
					else {
						return _command_set{ };
					}
				}),
				Context([](ContextStk& cstk, _command_set& _vec, Word& w,  auto* compiler) {	// op_end
					return _command_set{
						Command(OPERATOR::STRCAT)
					};
				}, Context_Type::END)
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
					if (w.getType() != WordType::NUMBER) {
						return _command_set{
							Command(OPERATOR::CAST_NUMBER)
						};
					}
					else {
						return _command_set{ };
					}
				}, Context_Type::NORMAL, "add_op1"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_2
					if (w.getType() != WordType::NUMBER) {
						return _command_set{
							Command(OPERATOR::CAST_NUMBER)
						};
					}
					else {
						return _command_set{ };
					}
				}, Context_Type::NORMAL, "add_op2"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_end
					return _command_set{
						Command(OPERATOR::ADD)
					};
				}, Context_Type::END, "add_end")
			})
	);

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
							Command(OPERATOR::CAST_NUMBER)
						};
					}
					else {
						return _command_set{ };
					}
				}),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {	// op_2
					if (w.getType() != WordType::NUMBER) {
						return _command_set{
							Command(OPERATOR::CAST_NUMBER)
						};
					}
					else {
						return _command_set{ };
					}
				}),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{
						Command(OPERATOR::SUB)
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
						Command(OPERATOR::CAST_NUMBER)
					};
				}),
				Context([](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) {
					return _command_set{
						Command(OPERATOR::NOT)
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
						Command(OPERATOR::EQ)
					};
				}, Context_Type::END)
			})
	);

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
						Command(OPERATOR::L)
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
							Command(OPERATOR::G)
						};
					}, Context_Type::END)
				})
	);

	helper.regist_context(Word(WordType::OPERATOR_WORD, "echo").serialize(),
		helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w,  auto* compiler) { return _command_set{}; },
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
	REGIST_TOKEN(helper, "//", "commetLine");

	// 
	REGIST_TOKEN(helper, " ", "space");
	REGIST_TOKEN(helper, "\t", "tab");
	REGIST_TOKEN(helper, "\n", "space");
	REGIST_TOKEN(helper, ":", "colon");
	REGIST_TOKEN(helper, ";", "context_closed");
	REGIST_TOKEN(helper, "*", "star");
	REGIST_TOKEN(helper, "<", "string_open");
	REGIST_TOKEN(helper, ">", "string_closed");
	REGIST_TOKEN(helper, "{", "node_open");
	REGIST_TOKEN(helper, "}", "node_closed");
	REGIST_TOKEN(helper, "[", "list_open");
	REGIST_TOKEN(helper, "]", "list_closed");
	REGIST_TOKEN(helper, "\\", "connectLine");
}

// 注册word
void regist_words(WordTypeHelper& helper) {
	REGIST_OPERATO_WORDS(helper, "add");
	REGIST_OPERATO_WORDS(helper, "sub");
	REGIST_OPERATO_WORDS(helper, "echo");
	REGIST_OPERATO_WORDS(helper, "strcat");
	REGIST_OPERATO_WORDS(helper, "not");
	REGIST_OPERATO_WORDS(helper, "eq");
	REGIST_OPERATO_WORDS(helper, "g");
	REGIST_OPERATO_WORDS(helper, "l");

	REGIST_CONTROLLER_WORDS(helper, "time");
	REGIST_CONTROLLER_WORDS(helper, "call");
	REGIST_CONTROLLER_WORDS(helper, "prcd");
	REGIST_CONTROLLER_WORDS(helper, "tuple");
	REGIST_CONTROLLER_WORDS(helper, "def");
	REGIST_CONTROLLER_WORDS(helper, "assign");
	REGIST_CONTROLLER_WORDS(helper, "ignore");
	REGIST_CONTROLLER_WORDS(helper, "while");
	REGIST_CONTROLLER_WORDS(helper, "rept");
	REGIST_CONTROLLER_WORDS(helper, "if");
	REGIST_CONTROLLER_WORDS(helper, "for");
	REGIST_CONTROLLER_WORDS(helper, "abort");
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
		else{ 
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
		else{ 
			err += "lexer pos: " + to_string(lex_point) + "\tmight be '[' here\n";
			return false;
		}
	})->
		nonterminal("controller", "controller")->
		nonterminal("controller", "controller")->
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
		else{
			err += "lexer pos: " + to_string(lex_point) + "\tmight be ']' here\n";
			return false;
		}
	})->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_block);

	Graphic_builder string_builder("string");
	BNFGraphic g_string = string_builder.rule()->
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
		else{
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
		else{
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
		else{
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
		else{
			err += "lexer pos: " + to_string(lex_point) + "\tmight be '>' here\n"; 
			return false;
		}
	})->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_string);

	Graphic_builder list_builder("list");
	BNFGraphic g_list =
		list_builder.rule()->
		terminal("key_word", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[key_word] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::OPERATOR_WORD) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else{
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
		else{
			err += "lexer pos: " + to_string(lex_point) + "\tmight be ';' here\n"; 
			return false;
		}
	})->
		end()->
		gotoHead()->
		nonterminal("def_ass", "def_ass")->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_list);

	Graphic_builder atomic_builder("atomic");
	BNFGraphic g_atomic =
		atomic_builder.rule()->
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
		else{
			err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " is not a 'NUMBER' word\n";
			return false;
		}
	})->
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
		nonterminal("list", "list")->
		end()->
		gotoHead()->
		nonterminal("string", "string")->
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
		getGraphic(); 
	p.addBNFRuleGraphic(g_atomic);

	// tuple 多返回值语句
	Graphic_builder tuple_builder("tuple");
	BNFGraphic g_tuple =
		tuple_builder.rule()->
		terminal("tuple", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[tuple] get word:" << w.serialize();
#endif
		if (w.getString() == "tuple") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else {
			err += "lexer pos: " + to_string(lex_point) + "\tmight be 'tuple' here\n";
			return false;
		}
	})->
		nonterminal("atomic", "atomic")->
		nonterminal("atomic", "atomic")->
		terminal("context_closed_tuple", [&](Word w, std::string& err, int lex_point) {
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
	p.addBNFRuleGraphic(g_tuple);

	// def和assign语句
	Graphic_builder def_ass_builder("def_ass");
	BNFGraphic g_def_ass = def_ass_builder.rule()->
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
		terminal("id_enabled_def", [&](Word w, std::string& err, int lex_point) {
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
		nonterminal("atomic", "atomic")->
		terminal("context_closed_def", [&](Word w, std::string& err, int lex_point) {
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
		// assign语句
		gotoHead()->
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
		terminal("id_enabled_assign", [&](Word w, std::string& err, int lex_point) {
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
		nonterminal("atomic", "atomic")->
		terminal("context_closed_assign", [&](Word w, std::string& err, int lex_point) {
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
		// prcd语句
		gotoHead()->
		terminal("prcd", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[prcd] get word:" << w.serialize();
#endif
			if (w.getString() == "prcd") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be 'prcd' here\n";
				return false;
			}
		})->
		nonterminal("block", "block")->
		end()->
		// time语句
		gotoHead()->
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
		nonterminal("block", "block")->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_def_ass);

	// 流程控制语句
	Graphic_builder controller_builder("controller");
	BNFGraphic g_controller =
		controller_builder.rule()->
		nonterminal("list", "list")->
		nonterminal("list", "list")->
		end()->
		// call 语句
		gotoHead()->
		terminal("call", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[call] get word:" << w.serialize();
#endif
			if (w.getString() == "call") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be 'call' here\n";
				return false;
			}
		})->
		nonterminal("atomic", "atomic")->
		nonterminal("atomic", "atomic")->
		terminal("context_closed_call", [&](Word w, std::string& err, int lex_point) {
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
		// for语句
		gotoHead()->
		terminal("for", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
			cerr << "[for] get word:" << w.serialize();
#endif
			if (w.getString() == "for") {
#if CHECK_Parser
				cerr << ", True" << endl;
#endif
				return true;
			}
			else {
				err += "lexer pos: " + to_string(lex_point) + "\tmight be 'for' here\n";
				return false;
			}
		})->
		nonterminal("tuple", "tuple")->
		terminal("id_enabled_for", [&](Word w, std::string& err, int lex_point) {
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
		nonterminal("block_for", "block")->
		end()->
		// rept语句
		gotoHead()->
		terminal("rept", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[rept] get word:" << w.serialize();
#endif
		if (w.getString() == "rept") {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else{
			err += "lexer pos: " + to_string(lex_point) + "\tmight be 'rept' here\n"; 
			return false;}
	})->
		nonterminal("atomic", "atomic")->
		nonterminal("block_rept", "block")->
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
			else{
				err += "lexer pos: " + to_string(lex_point) + "\tmight be 'if' here\n"; 
				return false;}
		})->
		nonterminal("atomic", "atomic")->
		nonterminal("block_if", "block")->
		end()->
		// ignore语句
		gotoHead()->
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
		else{
			err += "lexer pos: " + to_string(lex_point) + "\tmight be 'ignore' here\n"; 
			return false;}
	})->
		nonterminal("block_ignore", "block")->
		end()->
		// abort语句
		gotoHead()->
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
		else{
			err += "lexer pos: " + to_string(lex_point) + "\tmight be 'abort' here\n"; 
			return false;}
	})->
		nonterminal("atomic", "atomic")->
		terminal("context_closed_abort", [&](Word w, std::string& err, int lex_point) {
#if CHECK_Parser
		cerr << "[context_closed] get word:" << w.serialize();
#endif
		if (w.getType() == WordType::CONTEXT_CLOSED) {
#if CHECK_Parser
			cerr << ", True" << endl;
#endif
			return true;
		}
		else{ 
			err += "lexer pos: " + to_string(lex_point) + "\tmight be ']' here\n"; 
			return false;}
	})->
		end()->
		// while语句
		gotoHead()->
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
		else{
			err += "lexer pos: " + to_string(lex_point) + "\tmight be 'while' here\n"; 
			return false;}
	})->
		nonterminal("atomic", "atomic")->
		nonterminal("block_while", "block")->
		end()->
		getGraphic();
	p.addBNFRuleGraphic(g_controller);
}

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

int main(int argc, char* argv[]) {
	
	// 准备工作
	Context_helper ctx_helper;
	regist_keywords_contents(ctx_helper);
	
	Token_helper token_helper;
	regist_token(token_helper);

	WordTypeHelper word_type_helper;
	regist_words(word_type_helper);

	// 
	Lexer lex(new CLIInput("luo ->"));
	// Lexer lex(new FileInput("in.tr"));
	Parser p(&lex);
	regist_bnf(p);

	S_Expr_Compiler compiler;

	VirtualMachine vm;
	vector<Command> comms;
	while (1) {
		comms.clear();
		p.init();

		// 词法分析
		lex.init();
		lex.fillList(token_helper, word_type_helper);

#if CHECK_Lexer
		do {
			cerr << "[" << lex.peek().serialize() << "], " << endl;
		} while (lex.next());
#endif

		// 语法分析
		try {
			int b = p.parse("top");
			cerr << "Parse " << (b ? "success!" : "failed!") << endl;
			if (!b) {
				cout << "Parse ERROR =\n" << p.getErrors() << endl;
			}
			else {

				// 生成目标代码
				compiler.generate_code(p.get_word_vector_ref(), comms, ctx_helper);	// 可能抛出异常
				cout << "Code Generated finished!" << endl << endl;

				// 执行代码
				vm.setInstruct(comms);
				vm.run();
			}
		}
		catch (Context_found_error e) {
			cerr << e.what() << endl;
		}
	}

	return 0;
};