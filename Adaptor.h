#pragma once
class Adaptor
{
public:
	virtual std::vector<Word> adapt_code(std::vector<Word>& ws, WordTypeHelper& helper) = 0;
	Adaptor();
	~Adaptor();
};

// 中缀表达式转前缀表达式 转换器
class Mid_Expr_Adaptor : public Adaptor {
protected:
	// 显示两个栈的内容
	void show_rev(std::vector<Word>& s1, std::vector<Word>& s2, std::vector<int>& ps) {
		std::cout << "s1--> ";
		for (auto w : s1) {
			std::cout << w.getString() << std::ends;
		}
		std::cout << std::endl;
		std::cout << "s2--> ";
		for (auto w : s2) {
			std::cout << w.getString() << std::ends;
		}
		std::cout << std::endl;
		std::cout << "ps--> ";
		for (auto p : ps) {
			std::cout << p << std::ends;
		}
		std::cout << std::endl;
	}

	// 压入到S2
	void _push_into_s2(Word& w, std::vector<Word>& ws) {
		if (w.getType() != WordType::SEPARATOR)
			ws.push_back(w);
	}
public:
	// 输入打好标签的word数组, 返回前缀表达式
	virtual std::vector<Word> adapt_code(std::vector<Word>& ws, WordTypeHelper& helper) override;
};