#include "pch.h"
#include "Adaptor.h"

Adaptor::Adaptor()
{
}

Adaptor::~Adaptor()
{
}

// 输入打好标签的word数组, 返回前缀表达式
std::vector<Word> Mid_Expr_Adaptor::adapt_code(std::vector<Word>& ws, WordTypeHelper& helper)
{
	Word ctx_close(WordType::CONTEXT_CLOSED, ";");
	size_t size = ws.size();
	// 运算符栈s1
	std::vector<Word> s1;
	// 中间结果栈s2
	std::vector<Word> s2;
	// 结束符
	Word eos;
	// 从右至左扫描中缀表达式
	for (int i = size - 1; i >= 0; --i) {
		Word w = ws[i];
		auto type = w.getType();
		if (helper.is_opera2(w)) {
			// 是二元运算符
			int level = helper.get_level(w);
			while (1) {
				if (!s1.empty()) {
					// 非空, 比较其与S1栈顶运算符的优先级
					auto top_w = s1.back();
					auto top_type = top_w.getType();
					int top_level = helper.get_level(top_w);
					if (top_type == WordType::BRACKET_CLOSED) {
						// 栈顶运算符为右括号")", 直接将此运算符入栈
						s1.push_back(w);
						break;
					}
					else if (top_type == WordType::CONTEXT_CLOSED) {
						// 栈顶运算符为";", 直接将此运算符入栈
						s1.push_back(w);
						break;
					}
					else if (level >= top_level) {
						// 优先级比栈顶运算符的较高或相等, 将运算符压入S1
						s1.push_back(w);
						break;
					}
					else {
						// 将S1栈顶的运算符弹出并压入到S2中, w再次与S1中新的栈顶运算符相比较, 直到满足上面两个条件为止
						Word opera = s1.back();
						_push_into_s2(opera, s2);
						s1.pop_back();
					}
				}
				else {
					// 如果S1为空, 直接将此运算符入栈S1
					s1.push_back(w);
					break;
				}
			} // while end
		}
		else if (type == WordType::BRACKET_CLOSED) {
			// 括号), 直接压入S1
			s1.push_back(w);
		}
		else if (type == WordType::BRACKET_OPEN) {
			// 括号(, 依次弹出S1栈顶的运算符,并压入S2, 直到遇到右括号为止, 丢弃括号
			while (!s1.empty()) {
				Word opera = s1.back();
				if (opera.getType() == WordType::BRACKET_CLOSED) {
					s1.pop_back();
					break;
				}
				_push_into_s2(opera, s2);
				s1.pop_back();
			}
		}
		else if (type == WordType::CONTEXT_CLOSED) {
			// ;号, 直接压入S1, S2
			s1.push_back(w);
			_push_into_s2(w, s2);
		}
		else if (!helper.is_opera2(w) && (type == WordType::OPERATOR_WORD || type == WordType::CONTROLLER)) {
			// 遇到操作字符和控制符, 先放入S2栈, 之后依次弹出S1栈顶的运算符,并压入S2, 直到遇到;号为止
			while (!s1.empty()) {
				Word opera = s1.back();
				s1.pop_back();
				if (opera.getType() == WordType::CONTEXT_CLOSED) {
					break;
				}
				_push_into_s2(opera, s2);
			}
			_push_into_s2(w, s2);
		}
		else {
			// 遇到操作数时，将其压入S2
			_push_into_s2(w, s2);
		}
	}

	// 将S1中剩余的运算符依次弹出并压入S2
	while (!s1.empty()) {
		Word opera = s1.back();
		s1.pop_back();
		_push_into_s2(opera, s2);
	}

	// 依次弹出S2中的元素并输出，结果即为中缀表达式对应的前缀表达式
	std::vector<Word> res(s2.rbegin(), s2.rend());
	return res;
}
