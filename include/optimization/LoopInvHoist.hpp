#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "PassManager.hpp"
#include "Module.h"
#include "Function.h"
#include "BasicBlock.h"

class LoopInvHoist : public Pass
{
public:
    LoopInvHoist(Module *m) : Pass(m) {}

    void run() override;

private:
    // 记录深度优先遍历过程中已经访问过的基本块
    std::unordered_set<BasicBlock*> visited;
    // 深度优先遍历循环的每个基本块
    void dfs(BasicBlock* node, BBset_t *loop, LoopSearch loop_searcher);
    bool can_delete(Instruction *instr, BBset_t *loop, std::vector<Instruction *>wait_delete);
};
