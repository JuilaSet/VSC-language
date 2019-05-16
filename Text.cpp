#include "pch.h"
#include "Text.h"


/*
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
	Context_helper vsc_ctx_helper;
	regist_keywords_contents(vsc_ctx_helper);

	Token_helper vsc_token_helper;
	regist_token(vsc_token_helper);

	WordTypeHelper vsc_word_type_helper;
	regist_words(vsc_word_type_helper);

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
		lex.fillList(vsc_token_helper, vsc_word_type_helper);
		std::vector<Word>& words = lex.get_words_list();

		for (auto w : words) {
			cout << w.serialize() << ends;
		}

		cout << endl << "转为前缀表达式: " << endl;

		auto adapts = adptor.adapt_code(words, vsc_word_type_helper);
		for (auto w : adapts) {
			cout << w.getString() << ends;
		}
		cout << endl;
	}
}
*/