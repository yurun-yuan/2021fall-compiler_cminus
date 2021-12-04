#ifndef CONSTPROPAGATION_HPP
#define CONSTPROPAGATION_HPP
#include "PassManager.hpp"
#include "Constant.h"
#include "Instruction.h"
#include "Module.h"

#include "Value.h"
#include "IRBuilder.h"
#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>

class ConstPropagation : public Pass
{
public:
    ConstPropagation(Module *m) : Pass(m){}
    void run();

private:
    void replace_const(BasicBlock *);
    Value *const_compute(Instruction *instr);
    std::unordered_set<BasicBlock *> visited;
};

#endif
