#ifndef _CMINUSF_BUILDER_HPP_
#define _CMINUSF_BUILDER_HPP_
#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"
#include "ast.hpp"
#include <map>
#include <stack>
#include <utility>
#include <functional>
#include <algorithm>

using namespace std::placeholders;
using ConvertorFuncType = std::function<void(Value *&)>;
using CompFuncType = std::function<Value *(Value *left, Value *right)>;
using AddFuncType = std::function<Value *(Value *left, Value *right)>;
using MulFuncType = std::function<Value *(Value *left, Value *right)>;

#define MOD this->module.get()

#define GET_INT32 Type::get_int32_type(MOD)
#define GET_FLOAT Type::get_float_type(MOD)
#define GET_VOID Type::get_void_type(MOD)
#define GET_BOOL Type::get_int1_type(MOD)
#define CONST_INT(num) \
    (ConstantInt::get(num, MOD))
#define CONST_FP(num) \
    (ConstantFP::get(num, MOD))

#define GET_CONST(astNum) \
    ((astNum).type == TYPE_INT ? (Value *)CONST_INT((astNum).i_val) : (Value *)CONST_FP((astNum).f_val))

class Scope
{
public:
    // enter a new scope
    void enter()
    {
        inner.push_back({});
    }

    // exit a scope
    void exit()
    {
        inner.pop_back();
    }

    bool in_global()
    {
        return inner.size() == 1;
    }

    // push a name to scope
    // return true if successful
    // return false if this name already exits
    bool push(std::string name, Value *val)
    {
        auto result = inner.back().insert({name, val});
        return result.second;
    }

    Value *find(std::string name)
    {
        for (auto s = inner.rbegin(); s != inner.rend(); s++)
        {
            auto iter = s->find(name);
            if (iter != s->end())
            {
                return iter->second;
            }
        }
        return nullptr;
    }

private:
    std::vector<std::map<std::string, Value *>> inner;
};

class CminusfBuilder : public ASTVisitor
{
public:
    CminusfBuilder()
    {
        module = std::unique_ptr<Module>(new Module("Cminus code"));
        builder = new IRBuilder(nullptr, module.get());
        auto TyVoid = Type::get_void_type(module.get());
        auto TyInt32 = Type::get_int32_type(module.get());
        auto TyFloat = Type::get_float_type(module.get());

        auto input_type = FunctionType::get(TyInt32, {});
        auto input_fun =
            Function::create(
                input_type,
                "input",
                module.get());

        std::vector<Type *> output_params;
        output_params.push_back(TyInt32);
        auto output_type = FunctionType::get(TyVoid, output_params);
        auto output_fun =
            Function::create(
                output_type,
                "output",
                module.get());

        std::vector<Type *> output_float_params;
        output_float_params.push_back(TyFloat);
        auto output_float_type = FunctionType::get(TyVoid, output_float_params);
        auto output_float_fun =
            Function::create(
                output_float_type,
                "outputFloat",
                module.get());

        auto neg_idx_except_type = FunctionType::get(TyVoid, {});
        auto neg_idx_except_fun =
            Function::create(
                neg_idx_except_type,
                "neg_idx_except",
                module.get());

        scope.enter();
        scope.push("input", input_fun);
        scope.push("output", output_fun);
        scope.push("outputFloat", output_float_fun);
        scope.push("neg_idx_except", neg_idx_except_fun);

        compulsiveTypeConvertTable = {{{GET_INT32, GET_BOOL}, ConvertorFuncType([&](Value *&origin)
                                                                                { origin = builder->create_icmp_ne(origin, ConstantInt::get(0, MOD)); })},
                                      {{GET_FLOAT, GET_BOOL}, ConvertorFuncType([&](Value *&origin)
                                                                                { origin = builder->create_fcmp_ne(origin, ConstantFP::get(0, MOD)); })},
                                      {{GET_BOOL, GET_INT32}, ConvertorFuncType([&](Value *&origin)
                                                                                { origin = builder->create_zext(origin, GET_INT32); })},
                                      {{GET_FLOAT, GET_INT32}, ConvertorFuncType([&](Value *&origin)
                                                                                 { origin = builder->create_fptosi(origin, GET_INT32); })},
                                      {{GET_BOOL, GET_FLOAT}, ConvertorFuncType([&](Value *&origin)
                                                                                { origin = builder->create_sitofp(builder->create_zext(origin, GET_INT32), GET_FLOAT); })},
                                      {{GET_INT32, GET_FLOAT}, ConvertorFuncType([&](Value *&origin)
                                                                                 { origin = builder->create_sitofp(origin, GET_FLOAT); })}};
        typeOrderRank = {{GET_BOOL, 0},
                         {GET_INT32, 1},
                         {GET_FLOAT, 2}};
        compFuncTable = {
            {{GET_INT32, OP_LT}, [&](Value *left, Value *right)
             { return builder->create_icmp_lt(left, right); }},
            {{GET_INT32, OP_LE},
             [&](Value *left, Value *right)
             { return builder->create_icmp_le(left, right); }},
            {{GET_INT32, OP_EQ},
             [&](Value *left, Value *right)
             { return builder->create_icmp_eq(left, right); }},
            {{GET_INT32, OP_NEQ},
             [&](Value *left, Value *right)
             { return builder->create_icmp_ne(left, right); }},
            {{GET_INT32, OP_GT},
             [&](Value *left, Value *right)
             { return builder->create_icmp_gt(left, right); }},
            {{GET_INT32, OP_GE},
             [&](Value *left, Value *right)
             { return builder->create_icmp_ge(left, right); }},
            {{GET_FLOAT, OP_LT}, [&](Value *left, Value *right)
             { return builder->create_fcmp_lt(left, right); }},
            {{GET_FLOAT, OP_LE},
             [&](Value *left, Value *right)
             { return builder->create_fcmp_le(left, right); }},
            {{GET_FLOAT, OP_EQ},
             [&](Value *left, Value *right)
             { return builder->create_fcmp_eq(left, right); }},
            {{GET_FLOAT, OP_NEQ},
             [&](Value *left, Value *right)
             { return builder->create_fcmp_ne(left, right); }},
            {{GET_FLOAT, OP_GT},
             [&](Value *left, Value *right)
             { return builder->create_fcmp_gt(left, right); }},
            {{GET_FLOAT, OP_GE},
             [&](Value *left, Value *right)
             { return builder->create_fcmp_ge(left, right); }}};
        addFuncTable = {{{GET_INT32, OP_PLUS}, [&](Value *left, Value *right)
                         { return builder->create_iadd(left, right); }},
                        {{GET_INT32, OP_MINUS}, [&](Value *left, Value *right)
                         { return builder->create_isub(left, right); }},
                        {{GET_FLOAT, OP_PLUS}, [&](Value *left, Value *right)
                         { return builder->create_fadd(left, right); }},
                        {{GET_FLOAT, OP_MINUS}, [&](Value *left, Value *right)
                         { return builder->create_fsub(left, right); }}};
        mulFuncTable = {{{GET_INT32, OP_MUL}, [&](Value *left, Value *right)
                         { return builder->create_imul(left, right); }},
                        {{GET_INT32, OP_DIV}, [&](Value *left, Value *right)
                         { return builder->create_isdiv(left, right); }},
                        {{GET_FLOAT, OP_MUL}, [&](Value *left, Value *right)
                         { return builder->create_fmul(left, right); }},
                        {{GET_FLOAT, OP_DIV}, [&](Value *left, Value *right)
                         { return builder->create_fdiv(left, right); }}};
    }

    std::unique_ptr<Module>
    getModule()
    {
        return std::move(module);
    }

private:
    virtual void visit(ASTProgram &) override final;
    virtual void visit(ASTNum &) override final;
    virtual void visit(ASTVarDeclaration &) override final;
    virtual void visit(ASTFunDeclaration &) override final;
    virtual void visit(ASTParam &) override final;
    virtual void visit(ASTCompoundStmt &) override final;
    virtual void visit(ASTExpressionStmt &) override final;
    virtual void visit(ASTSelectionStmt &) override final;
    virtual void visit(ASTIterationStmt &) override final;
    virtual void visit(ASTReturnStmt &) override final;
    virtual void visit(ASTAssignExpression &) override final;
    virtual void visit(ASTSimpleExpression &) override final;
    virtual void visit(ASTAdditiveExpression &) override final;
    virtual void visit(ASTVar &) override final;
    virtual void visit(ASTTerm &) override final;
    virtual void visit(ASTCall &) override final;

    /**
     * @brief Convert `enum CminusType` to `Type *`
     * @arg CminusType: a `enum CminusType` value
     * @arg res: a `Type *` value. The converted value will be assigned to it.
     * 
     */
    Type *CminusTypeConvertor(enum CminusType c)
    {
        switch (c)
        {
        case TYPE_INT:
            return GET_INT32;
            break;
        case TYPE_FLOAT:
            return GET_FLOAT;
            break;
        case TYPE_VOID:
            return GET_VOID;
            break;
        default:
            break;
        }
    }

    bool isSelectStmt(ASTStatement *stmt)
    {
        return static_cast<bool>(dynamic_cast<ASTSelectionStmt *>(stmt));
    }

    bool isIterStmt(ASTStatement *stmt)
    {
        return static_cast<bool>(dynamic_cast<ASTIterationStmt *>(stmt));
    }

    bool isCtrlStmt(ASTStatement *stmt)
    {
        return isSelectStmt(stmt) || isIterStmt(stmt);
    }

    BasicBlock *newBasicBlock()
    {
        return BasicBlock::create(MOD, std::to_string(label_name_cnt++), lastEnteredFun);
    }

    std::map<std::pair<Type *, Type *>, ConvertorFuncType> compulsiveTypeConvertTable;

    void compulsiveTypeConvert(Value *&origin, Type *target)
    {
        if (origin->get_type() != target)
            compulsiveTypeConvertTable[{origin->get_type(), target}](origin);
    }

    Type *augmentTypeConvert(Value *&left, Value *&right)
    {
        auto resType = std::max(left->get_type(), right->get_type(), [&](Type *l, Type *r)
                                { return typeOrderRank[l] < typeOrderRank[r]; });
        compulsiveTypeConvert(left, resType);
        compulsiveTypeConvert(right, resType);
        return resType;
    }

    Type *augmentTypeConvert(Value *&left, Value *&right, Type *min)
    {
        auto resType = std::max({left->get_type(), right->get_type(), min}, [&](Type *l, Type *r)
                                { return typeOrderRank[l] < typeOrderRank[r]; });
        compulsiveTypeConvert(left, resType);
        compulsiveTypeConvert(right, resType);
        return resType;
    }

    Value* getArrOrPtrAddr(Value* var, Value* index){
        Value *obj_addr;
        if (var->get_type()->get_pointer_element_type()->is_array_type()) // is array
            obj_addr = builder->create_gep(var, {CONST_INT(0), index});
        else // is pointer
        {
            var = builder->create_load(var);
            obj_addr = builder->create_gep(var, {index});
        }
        return obj_addr;
    }

    std::map<Type *, int> typeOrderRank;
    std::map<std::pair<Type *, enum RelOp>, CompFuncType> compFuncTable;
    std::map<std::pair<Type *, enum AddOp>, AddFuncType> addFuncTable;
    std::map<std::pair<Type *, enum MulOp>, MulFuncType> mulFuncTable;

    IRBuilder *builder;

    /**
     * @brief The types in scope can be
     * AllocaInst
     * GlobalVariable
     * Function
     */
    Scope scope;
    std::unique_ptr<Module> module;
    Function *lastEnteredFun = nullptr;
    bool enteredFun = true;
    bool terminateStmt = false;
    std::vector<std::shared_ptr<ASTParam>> *params = nullptr;
    std::list<Argument *>::iterator curArg;
    // Use stack to evaluate expressions
    std::stack<Value *> cal_stack;
    std::stack<BasicBlock *> bb_stack;

    size_t label_name_cnt = 0;
};
#endif
