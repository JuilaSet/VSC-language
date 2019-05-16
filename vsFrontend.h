#pragma once
class vsFrontend
{
protected:
	// 所有的符号表
	Context_helper vsc_ctx_helper;
	Token_helper vsc_token_helper;
	WordTypeHelper vsc_word_type_helper;

	// 语法注册函数
	void (*regist_bnf)(Parser&);

	// 中间代码生成器
	S_Expr_Compiler compiler;

public:
	// 在构造函数中注册表, 指明输入的方式
	vsFrontend(
		void(*regist_bnf)(Parser&),
		void(*regist_keywords_contents)(Context_helper&), 
		void(*regist_token)(Token_helper&), 
		void(*regist_words)(WordTypeHelper&));

	// 生成编译结果
	Compile_result getResultByString(const std::string& codes, const std::string& top = "top");

};

