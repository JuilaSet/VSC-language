#pragma once

using pOperaFunc_t = std::function<void(vsEvaluator*)>;
class Command {
	friend class vsEvaluator;

#if CHECK_Eval_command
	std::string _name;
#endif

protected:
	friend class CommandHelper;
public:
	pOperaFunc_t opera;

#if CHECK_Eval_command
	Command(pOperaFunc_t func, const std::string name = "unknown") :opera(func), _name(name) { }
	inline const std::string& name() const {
		return _name;
	}
#else
	Command(pOperaFunc_t func) : opera(func) { }
#endif
};

using _command_set = std::vector<Command>;

// 用于调试模式下给command打上对应的标签
#if CHECK_Eval_command
#define COMMAND(name) Command(OPERATOR::name, #name)
#else
#define COMMAND(name) Command(OPERATOR::name)
#endif
//

// Command
class CommandHelper {
public:
	static Command getBasicCommandOfString(std::string str);

	static Command getBasicOpera(pOperaFunc_t func) {
		return Command(func);
	}

	static Command getPushOpera(data_ptr d) {
		// 传入值， 防止析构
		return Command([=](vsEvaluator *eval) {
#if CHECK_Eval 
			std::cerr << __LINE__ << "\tOPCODE::PUSH " << d->toEchoString() << std::endl;
#endif
			eval->push(d);
		}
#if CHECK_Eval_command
			, "PUSH " + d->toEchoString()
#endif 
			);
	};

	static Command getEchoOpera(std::ostream* ostm, std::string end = "\n") {
		return Command([=](vsEvaluator *eval) {
#if CHECK_Eval 
			std::cerr << __LINE__ << "\tOPCODE::ECHO ";
#endif
			auto& stk = eval->current_stk_frame()->stk;
			assert(!stk.empty());
			data_ptr d = eval->top();
			assert(d != nullptr);
			std::string str = d->toEchoString();
#if CHECK_Eval 
			std::cerr << str << std::endl;
#endif
			*ostm << str << end << std::flush;
		}
#if CHECK_Eval_command
			, "ECHO"
#endif 
			);
	}
};

