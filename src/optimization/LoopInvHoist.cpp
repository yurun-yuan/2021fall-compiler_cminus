#include <algorithm>
#include "logging.hpp"
#include "LoopSearch.hpp"
#include "LoopInvHoist.hpp"

void LoopInvHoist::run()
{
    // 先通过LoopSearch获取循环的相关信息
    LoopSearch loop_searcher(m_, false);
    loop_searcher.run();

    // 接下来由你来补充啦！
    // 获取函数列表
    auto func_list = m_->get_functions();
    // 遍历所有函数
    for (auto func : func_list)
    {
        if (func->get_basic_blocks().size() == 0)
        {
            continue;
        }
        else
        {
            // 获取函数的所有循环
            std::unordered_set<BBset_t *> loops = loop_searcher.get_loops_in_func(func);

            // 将循环按循环中块的个数排序，这样可以做到从最内存循环遍历所有循环
            std::vector<BBset_t *> loops_sorted{loops.begin(), loops.end()};
            std::sort(loops_sorted.begin(), loops_sorted.end(),
                      [&](BBset_t *lhs, BBset_t *rhs)
                      { return lhs->size() < rhs->size(); });

            for (auto loop = loops_sorted.begin(); loop != loops_sorted.end(); loop++)
            {
                // printf("DEBUG:%d\n", (int)(*loop)->size());
                auto base = loop_searcher.get_loop_base(*loop);
                visited.clear();
                dfs(base, (*loop), loop_searcher);
            }
        }
    }
}

void LoopInvHoist::dfs(BasicBlock *node, BBset_t *loop, LoopSearch loop_searcher)
{
    std::vector<Instruction *> wait_delete;
    visited.insert(node);
    // 遍历基本块中的所有指令
    for (auto instr = node->get_instructions().begin(); instr != node->get_instructions().end(); instr++)
    {
        if (can_delete(*instr, loop, wait_delete))
            wait_delete.push_back(*instr);
    }

    // 找到循环外提的目标块
    auto base = loop_searcher.get_loop_base(loop);
    BasicBlock *target;
    for (auto prev : base->get_pre_basic_blocks())
        if (loop->find(prev) == loop->end())
            target = prev;

    // 将可外提的指令外提到目标块中
    if (!wait_delete.empty())
    {
        // printf("DEBUG:can delete!\n");

        // 将指令直接插入目标块的beginning是错误的
        // 应该将指令插入目标块的branch指令之前
        auto branch = target->get_instructions().back();
        target->get_instructions().pop_back();
        for(auto instr : wait_delete)
        {
            target->add_instruction(instr);
            node->delete_instr(instr);
        }
        target->add_instruction(branch);
        wait_delete.clear();
        // for (auto instr = wait_delete.end() - 1; instr >= wait_delete.begin(); instr--)
        // {
        //     target->add_instr_begin(*instr);
        //     node->delete_instr(*instr);
        // }
    }

    // 接着深度优先遍历node的后继
    for (auto succ = node->get_succ_basic_blocks().begin(); succ != node->get_succ_basic_blocks().end(); succ++)
        if ((visited.find(*succ) == visited.end()) && (loop->find(*succ) != loop->end()))
            dfs(*succ, loop, loop_searcher);
}

bool LoopInvHoist::can_delete(Instruction *instr, BBset_t *loop, std::vector<Instruction *> wait_delete)
{
    auto to_instruction = [&](Value *v)
    { return dynamic_cast<Instruction *>(v); };
    auto are_instruction = [&](Value *lhs, Value *rhs)
    { return to_instruction(lhs) && to_instruction(rhs); };
    auto to_const = [&](Value *v)
    { return dynamic_cast<ConstantInt *>(v) || dynamic_cast<ConstantFP *>(v); };
    auto are_const = [&](Value *lhs, Value *rhs)
    { return to_const(lhs) && to_const(rhs); };

    // std::cout << "DEBUG:" << instr->get_instr_op_name() << std::endl;
    if (instr->isBinary() || instr->is_cmp() || instr->is_fcmp())
    {
        auto lhs = instr->get_operand(0);
        auto rhs = instr->get_operand(1);
        bool can_l = false, can_r = false;
        // 分别判断左值和右值是否满足可删除的条件
        if (to_const(lhs))
            can_l = true;
        if (to_instruction(lhs))
        {
            auto lparent = to_instruction(lhs)->get_parent();
            if (loop->find(lparent) == loop->end())
                can_l = true;
            else
            {
                for (auto del : wait_delete)
                {
                    if (del == to_instruction(lhs))
                        can_l = true;
                }
            }
        }

        if (to_const(rhs))
            can_r = true;
        if (to_instruction(rhs))
        {
            auto rparent = to_instruction(rhs)->get_parent();
            if (loop->find(rparent) == loop->end())
                can_r = true;
            else
            {
                for (auto del : wait_delete)
                {
                    if (del == to_instruction(lhs))
                        can_r = true;
                }
            }
        }

        return (can_l && can_r);
    } // if is_Binary
    else if(instr->is_fp2si() || instr->is_si2fp() || instr->is_zext())
    {
        auto lhs = instr->get_operand(0);
        bool can_l = false;
        if (to_const(lhs))
            can_l = true;
        if (to_instruction(lhs))
        {
            auto lparent = to_instruction(lhs)->get_parent();
            if (loop->find(lparent) == loop->end())
                can_l = true;
            else
            {
                for (auto del : wait_delete)
                {
                    if (del == to_instruction(lhs))
                        can_l = true;
                }
            }
        }
        return can_l;
    }
    else
        return false;
}