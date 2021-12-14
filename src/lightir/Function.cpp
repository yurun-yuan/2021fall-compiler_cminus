#include "Module.h"
#include "Function.h"
#include "IRprinter.h"

Function::Function(FunctionType *ty, const std::string &name, Module *parent)
    : Value(ty, name), parent_(parent), seq_cnt_(0)
{
    if (struct_return_type())
        return_value = new Argument(PointerType::get(get_return_type()), "ret", this);
    parent->add_function(this);
    build_args();
}

Function *Function::create(FunctionType *ty, const std::string &name, Module *parent)
{
    return new Function(ty, name, parent);
}

FunctionType *Function::get_function_type() const
{
    return static_cast<FunctionType *>(get_type());
}

Type *Function::get_return_type() const
{
    return get_function_type()->get_return_type();
}

Type *Function::get_print_type() const
{
    return PointerType::get(get_type());
}

Value *Function::get_ret() const
{
    return return_value;
}

unsigned Function::get_num_of_args() const
{
    return get_function_type()->get_num_of_args();
}

unsigned Function::get_num_basic_blocks() const
{
    return basic_blocks_.size();
}

Module *Function::get_parent() const
{
    return parent_;
}

void Function::remove(BasicBlock *bb)
{
    basic_blocks_.remove(bb);
    for (auto pre : bb->get_pre_basic_blocks())
        pre->remove_succ_basic_block(bb);
    for (auto succ : bb->get_succ_basic_blocks())
        succ->remove_pre_basic_block(bb);
}

void Function::build_args()
{
    auto *func_ty = get_function_type();
    unsigned num_args = get_num_of_args();
    auto struct_to_ptr = [&](Type *type)
    {
        if(type->is_struct_type())
            return static_cast<Type*>(PointerType::get(type));
        else
            return type;
    };
    for (int i = 0; i < num_args; i++)
        arguments_.push_back(new Argument(struct_to_ptr(func_ty->get_param_type(i)), "", this, i));
}

void Function::add_basic_block(BasicBlock *bb)
{
    basic_blocks_.push_back(bb);
}

void Function::set_instr_name()
{
    std::map<Value *, int> seq;
    for (auto arg : get_args())
    {
        if (seq.find(arg) == seq.end())
        {
            auto seq_num = seq.size() + seq_cnt_;
            if (arg->set_name("arg" + std::to_string(seq_num)))
            {
                seq.insert({arg, seq_num});
            }
        }
    }
    for (auto bb : basic_blocks_)
    {
        if (seq.find(bb) == seq.end())
        {
            auto seq_num = seq.size() + seq_cnt_;
            if (bb->set_name("label" + std::to_string(seq_num)))
            {
                seq.insert({bb, seq_num});
            }
        }
        for (auto instr : bb->get_instructions())
        {
            if (!instr->is_void() && seq.find(instr) == seq.end())
            {
                auto seq_num = seq.size() + seq_cnt_;
                if (instr->set_name("op" + std::to_string(seq_num)))
                {
                    seq.insert({instr, seq_num});
                }
            }
        }
    }
    seq_cnt_ += seq.size();
}

// Only used in definition and declaration
std::string Function::print()
{
    set_instr_name();
    std::string func_ir;
    if (is_declaration())
        func_ir += "declare ";
    else
        func_ir += "define ";

    if (struct_return_type())
        func_ir += "void";
    else
        func_ir += get_return_type()->print();
    func_ir += " ";
    func_ir += print_as_op(this, false);
    func_ir += "(";

    if (get_return_type()->is_struct_type())
    {
        assert(return_value);
        func_ir += return_value->print();
        func_ir += " sret";
        if (!is_declaration())
        {
            func_ir += " %";
            func_ir += return_value->get_name();
        }
        if (get_num_of_args() != 0)
            func_ir += ", ";
    }

    //print arg
    if (is_declaration())
    {
        for (int i = 0; i < get_num_of_args(); i++)
        {
            if (i)
                func_ir += ", ";
            func_ir += static_cast<FunctionType *>(get_type())->get_param_type(i)->print();
        }
    }
    else
    {
        for (auto arg = arg_begin(); arg != arg_end(); arg++)
        {
            if (arg != arg_begin())
            {
                func_ir += ", ";
            }
            func_ir += static_cast<Argument *>(*arg)->print();
        }
    }
    func_ir += ")";

    //print bb
    if (is_declaration())
    {
        func_ir += "\n";
    }
    else
    {
        func_ir += " {";
        func_ir += "\n";
        for (auto bb : get_basic_blocks())
        {
            func_ir += bb->print();
        }
        func_ir += "}";
    }
    return func_ir;
}

std::string Argument::print()
{
    std::string arg_ir;
    if (get_type()->is_struct_type())
        arg_ir += PointerType::get(get_type())->print();
    else
        arg_ir += get_type()->print();
    arg_ir += " %";
    arg_ir += get_name();
    return arg_ir;
}