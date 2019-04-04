#include "pch.h"
#include "vsVirtualMachine.h"

void vsVirtualMachine::run(vsEvaluator& eval) {
	eval.eval(this);
}
