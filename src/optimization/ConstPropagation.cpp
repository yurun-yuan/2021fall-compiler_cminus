#include "ConstPropagation.hpp"
#include "logging.hpp"
#include <iostream>

using namespace std;

Value *ConstPropagation::const_compute(Instruction *instr)
{
    // Dirty work
    auto to_const_int = [&](Value *v)
    { return dynamic_cast<ConstantInt *>(v); };
    auto to_const_fp = [&](Value *v)
    { return dynamic_cast<ConstantFP *>(v); };
    auto are_const_int = [&](Value *lhs, Value *rhs)
    { return to_const_int(lhs) && to_const_int(rhs); };
    auto are_const_fp = [&](Value *lhs, Value *rhs)
    { return to_const_fp(lhs) && to_const_fp(rhs); };

    // cout <<"-- "<< instr->get_name() << " " << instr->get_instr_op_name() << "\n";
    if (instr->isBinary())
    {
        auto lhs = instr->get_operand(0), rhs = instr->get_operand(1);
        if (are_const_int(lhs, rhs))
        {
            int c_value1 = to_const_int(lhs)->get_value();
            int c_value2 = to_const_int(rhs)->get_value();
            auto op = instr->get_instr_type();
            switch (op)
            {
            case Instruction::add:
                return ConstantInt::get(c_value1 + c_value2, m_);
                break;
            case Instruction::sub:
                return ConstantInt::get(c_value1 - c_value2, m_);
                break;
            case Instruction::mul:
                return ConstantInt::get(c_value1 * c_value2, m_);
                break;
            case Instruction::sdiv:
                return ConstantInt::get((int)(c_value1 / c_value2), m_);
                break;
            default:
                break;
            }
        }
        else if (are_const_fp(lhs, rhs))
        {
            float c_value1 = to_const_fp(lhs)->get_value();
            float c_value2 = to_const_fp(rhs)->get_value();
            auto op = instr->get_instr_type();
            switch (op)
            {
            case Instruction::fadd:
                return ConstantFP::get(c_value1 + c_value2, m_);
                break;
            case Instruction::fsub:
                return ConstantFP::get(c_value1 - c_value2, m_);
                break;
            case Instruction::fmul:
                return ConstantFP::get(c_value1 * c_value2, m_);
                break;
            case Instruction::fdiv:
                return ConstantFP::get(c_value1 / c_value2, m_);
                break;
            default:
                break;
            }
        }
        return nullptr;
    }
    else if (instr->is_cmp())
    {
        auto lhs = instr->get_operand(0), rhs = instr->get_operand(1);
        if (are_const_int(lhs, rhs))
        {
            int c_value1 = to_const_int(lhs)->get_value();
            int c_value2 = to_const_int(rhs)->get_value();
            auto op = dynamic_cast<CmpInst *>(instr)->get_cmp_op();
            switch (op)
            {
            case CmpInst::EQ:
                return ConstantInt::get(c_value1 == c_value2, m_);
                break;
            case CmpInst::NE:
                return ConstantInt::get(c_value1 != c_value2, m_);
                break;
            case CmpInst::GT:
                return ConstantInt::get(c_value1 > c_value2, m_);
                break;
            case CmpInst::GE:
                return ConstantInt::get(c_value1 >= c_value2, m_);
                break;
            case CmpInst::LT:
                return ConstantInt::get(c_value1 < c_value2, m_);
                break;
            case CmpInst::LE:
                return ConstantInt::get(c_value1 <= c_value2, m_);
                break;
            default:
                break;
            }
        }
        return nullptr;
    }
    else if (instr->is_fcmp())
    {
        auto lhs = instr->get_operand(0), rhs = instr->get_operand(1);
        if (are_const_fp(lhs, rhs))
        {
            float c_value1 = to_const_fp(lhs)->get_value();
            float c_value2 = to_const_fp(rhs)->get_value();
            auto op = dynamic_cast<FCmpInst *>(instr)->get_cmp_op();
            switch (op)
            {
            case FCmpInst::EQ:
                return ConstantInt::get(c_value1 == c_value2, m_);
                break;
            case FCmpInst::NE:
                return ConstantInt::get(c_value1 != c_value2, m_);
                break;
            case FCmpInst::GT:
                return ConstantInt::get(c_value1 > c_value2, m_);
                break;
            case FCmpInst::GE:
                return ConstantInt::get(c_value1 >= c_value2, m_);
                break;
            case FCmpInst::LT:
                return ConstantInt::get(c_value1 < c_value2, m_);
                break;
            case FCmpInst::LE:
                return ConstantInt::get(c_value1 <= c_value2, m_);
                break;
            default:
                break;
            }
        }
        return nullptr;
    }
    else if (instr->is_fp2si())
    {
        auto c_value = to_const_fp(instr->get_operand(0));
        if (c_value)
        {
            return ConstantInt::get(static_cast<int>(c_value->get_value()), m_);
        }
        return nullptr;
    }
    else if (instr->is_si2fp())
    {
        auto c_value = to_const_int(instr->get_operand(0));
        if (c_value)
        {
            return ConstantFP::get(static_cast<float>(c_value->get_value()), m_);
        }
        return nullptr;
    }
    else if (instr->is_zext())
    {
        auto c_value = to_const_int(instr->get_operand(0));
        if (c_value)
        {
            return ConstantInt::get(c_value->get_value(), m_);
        }
        return nullptr;
    }
    else
        return nullptr;
}

void ConstPropagation::run()
{
    for (auto f : m_->get_functions())
    {
        if (f->get_basic_blocks().size() > 0)
        {
            visited.clear();
            replace_const(f->get_entry_block());
        }
    }
}

void ConstPropagation::replace_const(BasicBlock *bb)
{
    visited.insert(bb);
    std::vector<Instruction *> wait_delete;
    BasicBlock *to_add_truebb = nullptr;
    unordered_map<Value *, Value *> latest_val;
    for (auto instr : bb->get_instructions())
    {
        if (instr->is_br())
        {
            auto br_instr = dynamic_cast<BranchInst *>(instr);
            if (br_instr->is_cond_br())
            {
                auto cond = dynamic_cast<ConstantInt *>(br_instr->get_operand(0));
                if (cond)
                {
                    auto cond_val = cond->get_value();
                    assert("The value of i1 should be either 1 or 0" && (cond_val == 1 || cond_val == 0));
                    BasicBlock *goto_bb, *other_bb;
                    goto_bb = dynamic_cast<BasicBlock *>(br_instr->get_operand(2 - cond_val));
                    other_bb = dynamic_cast<BasicBlock *>(br_instr->get_operand(cond_val + 1));
                    to_add_truebb = goto_bb;
                    wait_delete.push_back(instr);
                    bb->remove_succ_basic_block(other_bb);
                    other_bb->remove_pre_basic_block(bb);
                }
            }
        }
        else if (instr->is_store())
            latest_val[instr->get_operand(1)] = instr->get_operand(0);
        else if (instr->is_load())
        {
            if (latest_val.find(instr->get_operand(0)) == latest_val.end())
                latest_val[instr->get_operand(0)] = instr;
            else
            {
                instr->replace_all_use_with(latest_val[instr->get_operand(0)]);
                wait_delete.push_back(instr);
            }
        }
        else
        {
            auto res = const_compute(instr);
            if (res)
            {
                instr->replace_all_use_with(res);
                wait_delete.push_back(instr);
            }
        }
    }
    for (auto &&instr : wait_delete)
        bb->delete_instr(instr);
    if (to_add_truebb)
        BranchInst::create_br(to_add_truebb, bb);

    for (auto succ : bb->get_succ_basic_blocks())
        if (visited.find(succ) == visited.end())
            replace_const(succ);
}
