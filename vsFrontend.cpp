#include "pch.h"
#include "vsFrontend.h"

// 在构造函数中注册表
vsFrontend::vsFrontend(
	void(*regist_bnf)(Parser&),
	void(*regist_keywords_contents)(Context_helper&), 
	void(*regist_token)(Token_helper&), 
	void(*regist_words)(WordTypeHelper&)) 
	:regist_bnf(regist_bnf)
{
	// 注册符号表
	regist_keywords_contents(vsc_ctx_helper);
	regist_token(vsc_token_helper);
	regist_words(vsc_word_type_helper);
}


// 生成编译结果
Compile_result vsFrontend::getResultByString(const std::string& codes, const std::string& top)
 throw(Context_error, run_time_exception, ParserFailed) {

	// 通过字符串获取对象
	Lexer vscLexer(new StringInput(codes));

	// 编译结果
	Compile_result cresult;

	// 初始化语法分析器
	Parser parser(&vscLexer);
	
	// 注册语法
	regist_bnf(parser);

	// 词法分析
	vscLexer.init();
	vscLexer.fillList(vsc_token_helper, vsc_word_type_helper);

	// 语法分析
	int b = parser.parse(top);
	std::cerr << (b ? "" : "Parse failed!\n");
	if (!b) {
		// 分析出错, 返回错误
		std::cerr << "Parse ERROR =\n" << parser.getErrors() << std::endl;
		throw ParserFailed("Parse failed");
	}
	else
	{
		// 生成目标代码
		compiler.generate_code(parser.get_word_vector_ref(), cresult, vsc_ctx_helper);	// 会抛出异常
//		std::cout << "Code Generated finished!" << std::endl << std::endl;
	}

	// 返回结果
	compiler.init();
	return cresult;
}
