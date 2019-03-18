// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <regex>
using namespace std;

// 注册关键字到上下文的关联
void regist_keywords_contents() {
	const std::string PARA_WORD = "paras_";

	// CONTROLLER
	Context_Helper::helper.regist_context(Word(WordType::CONTROLLER, "call").serialize(),
		Context_Helper::helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
			p->paras_begin();
			return _command_set{ };
		},
		{
			EMPTY_CONTEXT,
			Context([=](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
				std::string n = std::to_string(p->paras()++);
				return _command_set{
						CommandHelper::getDefOpera(PARA_WORD + n)
				};
			}, Context_Type::ALWAYS), 
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
				p->paras_end();
				return _command_set{
						Command(OPERATOR::CALL)
				};
			}, Context_Type::END)
		})
	);

	Context_Helper::helper.regist_context(Word(WordType::CONTROLLER, "time").serialize(),
		Context_Helper::helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
		Context_Helper::helper.push_command_index(_vec.size());
			return _command_set{
					Command(OPERATOR::TIME_BEGIN)
			};
		},
		{
			Context([&](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
				return _command_set{ };
			}, Context_Type::NORMAL, "time_block"),
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
				return _command_set{
						Command(OPERATOR::TIME_END)
				};
			}, Context_Type::END, "time_end")
		})
	);

	Context_Helper::helper.regist_context(Word(WordType::CONTROLLER, "prcd").serialize(),
		Context_Helper::helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
			Context_Helper::helper.push_command_index(_vec.size());
			return _command_set{
					Command(OPERATOR::ERROR),	// 待确定
					Command(OPERATOR::JMP)
			};
		},
		{
			Context([&](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
				return _command_set{ };
			}, Context_Type::NORMAL, "prcd_op1"),
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
				int last_index = Context_Helper::helper.pop_command_index();
				int addr = _vec.size();
				_vec[last_index] = CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, addr + 1));
				return _command_set{
						Command(OPERATOR::RET),
						CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, last_index + 2))	// push过程的首地址
				};
			}, Context_Type::END, "prcd_end")
		})
	);

	Context_Helper::helper.regist_context(Word(WordType::CONTROLLER, "tuple").serialize(),
		Context_Helper::helper.build_context(
		[](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {	// op_0
			return _command_set{ CommandHelper::getPushOpera(Data()) };	// 存入void
		},
		{
			Context([](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {	// op_always
				return _command_set{ };
			}, Context_Type::ALWAYS, "tuple_op1"),
			Context([](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {	// op_end
				return _command_set{ };
			}, Context_Type::END, "tuple_end")
		})
	);

	Context_Helper::helper.regist_context(Word(WordType::CONTROLLER, "for").serialize(),
		Context_Helper::helper.build_context(
			[&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
				return _command_set{};
			},
			{
				// tuple位置
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
					Context_Helper::helper.push_command_index(_vec.size());
					Context_Helper::helper.push_command_index(_vec.size() + 1);
					p->in_def();
					return _command_set{
						Command(OPERATOR::ISNON),
						Command(OPERATOR::ERROR),		// 待确定
						Command(OPERATOR::JMP_TRUE)
					};
				}, Context_Type::NORMAL, "for_tuple"),
				// 标识符
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
					p->out_def();
					p->insert_local(w, WordType::IDENTIFIER);	// 告知parser声明过了第一个参数
					return _command_set{
						Command(OPERATOR::REVERSE_TOP),
						Command(OPERATOR::DEF),
						Command(OPERATOR::POP)
					};
				}, Context_Type::NORMAL, "for_op2"),
				// 结束
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
					int last_index = Context_Helper::helper.pop_command_index();
					int head_index = Context_Helper::helper.pop_command_index();
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

	Context_Helper::helper.regist_context(Word(WordType::CONTROLLER, "assign").serialize(),
		Context_Helper::helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
				p->in_def();
				return _command_set{};
			},
			{
				Context([&](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
					p->out_def();
					p->insert_local(w, WordType::IDENTIFIER);	// 告知parser声明过了第一个参数
					return _command_set{};
				}),
				EMPTY_CONTEXT,
				Context([&](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
					return _command_set{
						Command(OPERATOR::ASSIGN)
					};
				}, Context_Type::END)
			})
	);

	Context_Helper::helper.regist_context(Word(WordType::CONTROLLER, "def").serialize(),
		Context_Helper::helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
				p->in_def();
				return _command_set{};
			},
			{
				Context([&](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
					p->out_def();
					p->insert_local(w, WordType::IDENTIFIER);	// 告知parser声明过了第一个参数
					return _command_set{};
				}),
				EMPTY_CONTEXT,
				Context([&](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
					return _command_set{
						Command(OPERATOR::DEF)
					};
				}, Context_Type::END)
			})
	);

	Context_Helper::helper.regist_context(Word(WordType::CONTROLLER, "abort").serialize(),
		Context_Helper::helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) { return _command_set{}; },
			{
				Context([&](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
					return _command_set{
						Command(OPERATOR::ABORT)
					};
				}, Context_Type::END)
			})
	);

	Context_Helper::helper.regist_context(Word(WordType::CONTROLLER, "ignore").serialize(),
		Context_Helper::helper.build_context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
			Context_Helper::helper.push_command_index(_vec.size());
			return _command_set{
					Command(OPERATOR::ERROR),	// 待确定
					Command(OPERATOR::JMP)
			};
		},
		{
			EMPTY_CONTEXT,
			Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
				int last_index = Context_Helper::helper.pop_command_index();
				int addr = _vec.size();
				_vec[last_index] = CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, addr));
				return _command_set{
						Command(OPERATOR::NOP)
				};
			}, Context_Type::END)
		})
	);

	Context_Helper::helper.regist_context(Word(WordType::CONTROLLER, "if").serialize(),
		Context_Helper::helper.build_context(
			[&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) { return _command_set{}; },
			{
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
					Context_Helper::helper.push_command_index(_vec.size() + 1);
					return _command_set{
						Command(OPERATOR::TEST),
						Command(OPERATOR::ERROR),		// 待确定
						Command(OPERATOR::JMP_FALSE)
					};
				}, Context_Type::NORMAL, "if_p1"),
				EMPTY_CONTEXT,
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
					int last_index = Context_Helper::helper.pop_command_index();
					int addr = _vec.size();
					_vec[last_index] = CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, addr));
					return _command_set{
						Command(OPERATOR::NOP)
					};
				}, Context_Type::END, "if_end")
			})
	);

	Context_Helper::helper.regist_context(Word(WordType::CONTROLLER, "while").serialize(),
		Context_Helper::helper.build_context(
			[&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
				Context_Helper::helper.push_command_index(_vec.size());
				return _command_set{};
			},
			{
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
					Context_Helper::helper.push_command_index(_vec.size() + 1);
					return _command_set{
						Command(OPERATOR::TEST),
						Command(OPERATOR::ERROR),		// 待确定
						Command(OPERATOR::JMP_FALSE)
					};
				}),
				EMPTY_CONTEXT,
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
					int last_index = Context_Helper::helper.pop_command_index();
					int head_index = Context_Helper::helper.pop_command_index();
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

	Context_Helper::helper.regist_context(Word(WordType::CONTROLLER, "rept").serialize(),
		Context_Helper::helper.build_context(
			[&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {	return _command_set{}; },
			{
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
					Context_Helper::helper.push_command_index(_vec.size() + 1);
					return _command_set {
						Command(OPERATOR::COUNT),
						Command(OPERATOR::NOP)
					};
				}),
				// block
				EMPTY_CONTEXT,
				Context([&](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {
					int addr = Context_Helper::helper.pop_command_index();
					return _command_set{
						CommandHelper::getPushOpera(Data(DataType::OPERA_ADDR, addr)),
						Command(OPERATOR::REPT)};
				}, Context_Type::END)
			})
	);
	
	// IDENTIFIER_SPEC
	
	Context_Helper::helper.regist_context(Word(WordType::IDENTIFIER_SPEC, "$C").serialize(),
		Context_Helper::helper.build_context(
		[](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {	// op_0
#if CHECK_Parser
		cout << "COMMAND:::: $C" << endl;
#endif
			return _command_set{ Command(OPERATOR::ECX) };
		},{ })	// 只能有一个上下文
	);

	Context_Helper::helper.regist_context(Word(WordType::IDENTIFIER_SPEC, "$null").serialize(),
		Context_Helper::helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {	// op_0
#if CHECK_Parser
		cout << "COMMAND:::: $null" << endl;
#endif
		return _command_set{ Command(OPERATOR::NUL) };
	}, { })	// 只能有一个上下文
	);

	// OPERATOR_WORD

	Context_Helper::helper.regist_context(Word(WordType::OPERATOR_WORD, "strcat").serialize(),
		Context_Helper::helper.build_context(
			[](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {	// op_0
				return _command_set{ };
			},
			{
				// 转为字符串
				Context([](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {	// op_1
					if (w.getType() != WordType::STRING) {
						return _command_set{
							Command(OPERATOR::CAST_STRING)
						};
					}
					else {
						return _command_set{ };
					}
				}),
				Context([](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {	// op_2
					if (w.getType() != WordType::STRING) {
						return _command_set{
							Command(OPERATOR::CAST_STRING)
						};
					}
					else {
						return _command_set{ };
					}
				}),
				Context([](ContextStk& cstk, _command_set& _vec, Word& w,  Compiler*  p) {	// op_end
					return _command_set{
						Command(OPERATOR::STRCAT)
					};
				}, Context_Type::END)
			})
	);

	Context_Helper::helper.regist_context(Word(WordType::OPERATOR_WORD, "add").serialize(),
		Context_Helper::helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {	// op_0
				return _command_set{ };
			},
			{
				// 转为数字
				Context([](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {	// op_1
					if (w.getType() != WordType::NUMBER) {
						return _command_set{
							Command(OPERATOR::CAST_NUMBER)
						};
					}
					else {
						return _command_set{ };
					}
				}, Context_Type::NORMAL, "add_op1"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {	// op_2
					if (w.getType() != WordType::NUMBER) {
						return _command_set{
							Command(OPERATOR::CAST_NUMBER)
						};
					}
					else {
						return _command_set{ };
					}
				}, Context_Type::NORMAL, "add_op2"),
				Context([](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {	// op_end
					return _command_set{
						Command(OPERATOR::ADD)
					};
				}, Context_Type::END, "add_end")
			})
	);

	Context_Helper::helper.regist_context(Word(WordType::OPERATOR_WORD, "sub").serialize(),
		Context_Helper::helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
				return _command_set{ };
			},
			{
				// 转为数字
				Context([](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {	// op_1
					if (w.getType() != WordType::NUMBER) {
						return _command_set{
							Command(OPERATOR::CAST_NUMBER)
						};
					}
					else {
						return _command_set{ };
					}
				}),
				Context([](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {	// op_2
					if (w.getType() != WordType::NUMBER) {
						return _command_set{
							Command(OPERATOR::CAST_NUMBER)
						};
					}
					else {
						return _command_set{ };
					}
				}),
				Context([](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
					return _command_set{
						Command(OPERATOR::SUB)
					};
				}, Context_Type::END)
			})
	);

	Context_Helper::helper.regist_context(Word(WordType::OPERATOR_WORD, "not").serialize(),
		Context_Helper::helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
				return _command_set{ };
			},
			{
				// 转为数字
				Context([](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {	// op_1
					return _command_set{
						Command(OPERATOR::CAST_NUMBER)
					};
				}),
				Context([](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
					return _command_set{
						Command(OPERATOR::NOT)
					};
				}, Context_Type::END)
			})
	);

	Context_Helper::helper.regist_context(Word(WordType::OPERATOR_WORD, "eq").serialize(),
		Context_Helper::helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
				return _command_set{ };
			},
			{
				EMPTY_CONTEXT,
				EMPTY_CONTEXT,
				Context([](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
					return _command_set{
						Command(OPERATOR::EQ)
					};
				}, Context_Type::END)
			})
	);

	Context_Helper::helper.regist_context(Word(WordType::OPERATOR_WORD, "l").serialize(),
		Context_Helper::helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
				return _command_set{ };
			},
			{
				EMPTY_CONTEXT,
				EMPTY_CONTEXT,
				Context([](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
					return _command_set{
						Command(OPERATOR::L)
					};
				}, Context_Type::END)
			})
	);

	Context_Helper::helper.regist_context(Word(WordType::OPERATOR_WORD, "g").serialize(),
		Context_Helper::helper.build_context(
			[](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
				return _command_set{ };
			},
			{
					EMPTY_CONTEXT,
					EMPTY_CONTEXT,
					Context([](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
						return _command_set{
							Command(OPERATOR::G)
						};
					}, Context_Type::END)
				})
	);

	Context_Helper::helper.regist_context(Word(WordType::OPERATOR_WORD, "echo").serialize(),
		Context_Helper::helper.build_context(
			[&](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) { return _command_set{}; },
			{
				Context([&](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
#if CHECK_Parser
					cout << "COMMAND ECHO" << endl;
#endif
					return _command_set{
						CommandHelper::getEchoOpera(&cout)
					};
				}, Context_Type::ALWAYS, "echo_op*"),
				Context([&](ContextStk& cstk, _command_set&, Word& w,  Compiler*  p) {
					return _command_set{ };
				}, Context_Type::END, "echo_end")
			})
	);
}

// 注册各种符号
void regist_token() {
	REGIST_TOKEN(" ", space);
	REGIST_TOKEN("\t", tab);
	REGIST_TOKEN("\n", space);
	REGIST_TOKEN(":", colon);
	REGIST_TOKEN(";", context_closed);
	REGIST_TOKEN("*", star);
	REGIST_TOKEN("<", string_open);
	REGIST_TOKEN(">", string_closed);
	REGIST_TOKEN("{", node_open);
	REGIST_TOKEN("}", node_closed);
	REGIST_TOKEN("[", list_open);
	REGIST_TOKEN("]", list_closed);
	REGIST_TOKEN("\\", connectLine);
	
	REGIST_OPERATO_WORDS("add");
	REGIST_OPERATO_WORDS("sub");
	REGIST_OPERATO_WORDS("echo");
	REGIST_OPERATO_WORDS("strcat");
	REGIST_OPERATO_WORDS("not");
	REGIST_OPERATO_WORDS("eq");
	REGIST_OPERATO_WORDS("g");
	REGIST_OPERATO_WORDS("l");

	REGIST_CONTROLLER_WORDS("time");
	REGIST_CONTROLLER_WORDS("call");
	REGIST_CONTROLLER_WORDS("prcd");
	REGIST_CONTROLLER_WORDS("tuple");
	REGIST_CONTROLLER_WORDS("def");
	REGIST_CONTROLLER_WORDS("assign");
	REGIST_CONTROLLER_WORDS("ignore");
	REGIST_CONTROLLER_WORDS("while");
	REGIST_CONTROLLER_WORDS("rept");
	REGIST_CONTROLLER_WORDS("if");
	REGIST_CONTROLLER_WORDS("for");
	REGIST_CONTROLLER_WORDS("abort");
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
			err += "lexer pos: " + to_string(lex_point) + "\t" + w.getString() + " is not an 'NUMBER' word\n";
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

int main(int argc, char* argv[]) {
	regist_keywords_contents();
	regist_token();

	Lexer lex(new CLIInput("luo ->"));
	//Lexer lex(new FileInput("in.tr"));
	Parser p(&lex);
	regist_bnf(p);

	Compiler compiler;

	VirtualMachine vm;
	vector<Command> comms;
	while (1) {
		comms.clear();
		p.init();

		// 词法分析
		lex.init();
		lex.fillList();

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
				compiler.generate_code(p.get_word_vector_ref(), comms, Context_Helper::helper);	// 可能抛出异常
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


// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
