#ifndef _CMINUSF_BUILDER_HPP_
#define _CMINUSF_BUILDER_HPP_
#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"
#include "ast.hpp"
#include "logging.hpp"
#include <map>
#include <stack>
#include <utility>
#include <functional>
#include <algorithm>
#include <optional>
#include <string>

// use these macros to get constant value

#define MOD builder->get_module()

// Macros for Type
#define GET_INT32 Type::get_int32_type(MOD)
#define GET_FLOAT Type::get_float_type(MOD)
#define GET_VOID Type::get_void_type(MOD)
#define GET_BOOL Type::get_int1_type(MOD)

#define CONST_INT(num) \
    (ConstantInt::get(num, MOD))
#define CONST_FP(num) \
    (ConstantFP::get(num, MOD))

#define CONST_ZERO(type) \
    ConstantZero::get(type, MOD)

#define INIT(Ty) ConstantZero::get((Ty), MOD)

struct ValueInfo
{
    Value *value = nullptr;
    bool is_lvalue = false;

    // Only for member functions
    struct
    {
        bool is_member_func = false;
        Value *belonged_struct_ptr;
    } struct_member;

    bool has_value()
    {
        return value;
    }
    Type *get_type()
    {
        if (is_lvalue)
        {
            assert(value->get_type()->is_pointer_type());
            return value->get_type()->get_pointer_element_type();
        }
        else
            return value->get_type();
    }
};

class Scope
{
public:
    void enter() { inner.push_back({}); }
    void exit() { inner.pop_back(); }
    bool in_global() { return inner.size() == 1; }
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
                return iter->second;
        }
        return nullptr;
    }

private:
    std::vector<std::map<std::string, Value *>> inner;
};

template <class T>
T pop(stack<T> &s)
{
    auto val = s.top();
    s.pop();
    return val;
}

template <class T>
void push(stack<T> &s)
{
    s.push(T{});
}

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

        scope.enter();
        scope.push("input_int", Function::create(
                                    FunctionType::get(TyInt32, {}, MOD),
                                    "input_int",
                                    module.get()));
        scope.push("input_float", Function::create(
                                      FunctionType::get(TyFloat, {}, MOD),
                                      "input_float",
                                      module.get()));
        scope.push("input_char", Function::create(
                                     FunctionType::get(TyInt32, {}, MOD),
                                     "input_char",
                                     module.get()));
        scope.push("print_int", Function::create(
                                    FunctionType::get(TyVoid, {TyInt32}, MOD),
                                    "print_int",
                                    module.get()));
        scope.push("println_int", Function::create(
                                      FunctionType::get(TyVoid, {TyInt32}, MOD),
                                      "println_int",
                                      module.get()));
        scope.push("print_float", Function::create(
                                      FunctionType::get(TyVoid, {TyFloat}, MOD),
                                      "print_float",
                                      module.get()));
        scope.push("println_float", Function::create(
                                        FunctionType::get(TyVoid, {TyFloat}, MOD),
                                        "println_float",
                                        module.get()));
        scope.push("print_char", Function::create(
                                     FunctionType::get(TyVoid, {TyInt32}, MOD),
                                     "print_char",
                                     module.get()));

        scope.push("allocate", Function::create(
                                   FunctionType::get(PointerType::get(TyInt32), {TyInt32}, MOD),
                                   "allocate",
                                   module.get()));
        scope.push("deallocate", Function::create(
                                     FunctionType::get(TyVoid, {PointerType::get(TyInt32)}, MOD),
                                     "deallocate",
                                     module.get()));

        // Initialize function tables
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

    std::unique_ptr<Module> getModule()
    {
        return std::move(module);
    }

private:
    using ConvertorFuncType = std::function<void(Value *&)>;
    using CompFuncType = std::function<Value *(Value *left, Value *right)>;
    using AddFuncType = std::function<Value *(Value *left, Value *right)>;
    using MulFuncType = std::function<Value *(Value *left, Value *right)>;

    virtual void visit(ASTProgram &) override final;
    virtual void visit(ASTVarDeclaration &) override final;
    virtual void visit(ASTDeclarationIdentifier &) override final;
    virtual void visit(ASTDeclarationDereference &) override final;
    virtual void visit(ASTDeclarationCall &) override final;
    virtual void visit(ASTDeclarationSubscript &) override final;
    virtual void visit(ASTFunDefinition &) override final;
    virtual void visit(ASTVarDefinition &) override final;
    virtual void visit(ASTClassTemplateDeclaration &) override final;
    virtual void visit(ASTStructSpecification &) override final;
    virtual void visit(ASTNamedType &) override final;
    virtual void visit(ASTCompoundStmt &) override final;
    virtual void visit(ASTExpressionStmt &) override final;
    virtual void visit(ASTSelectionStmt &) override final;
    virtual void visit(ASTIterationStmt &) override final;
    virtual void visit(ASTReturnStmt &) override final;
    virtual void visit(ASTVar &) override final;
    virtual void visit(ASTNum &) override final;
    virtual void visit(ASTReinterpretCast &) override final;
    virtual void visit(ASTCall &) override final;
    virtual void visit(ASTSubscript &) override final;
    virtual void visit(ASTMemberAccess &) override final;
    virtual void visit(ASTUnaryAddExpression &) override final;
    virtual void visit(ASTDereference &) override final;
    virtual void visit(ASTAddressof &) override final;
    virtual void visit(ASTMultiplicativeExpression &) override final;
    virtual void visit(ASTAdditiveExpression &) override final;
    virtual void visit(ASTRelationalExpression &) override final;
    virtual void visit(ASTAssignExpression &) override final;

    IRBuilder *builder;
    Scope scope;
    std::unique_ptr<Module> module;

    // ======================================================================

    std::map<Type *, int> typeOrderRank;
    std::map<std::pair<Type *, enum RelOp>, CompFuncType> compFuncTable;
    std::map<std::pair<Type *, enum AddOp>, AddFuncType> addFuncTable;
    std::map<std::pair<Type *, enum MulOp>, MulFuncType> mulFuncTable;
    std::map<std::pair<Type *, Type *>, ConvertorFuncType> compulsiveTypeConvertTable;

    // Operator overload
    std::map<std::pair<char, Type *>, Function *> operator_overload_table;

    // ======================================================================
    /**
     * @brief Pass values between `visit`s
     * 
     */

    bool compound_stmt_is_func_body = true;

    bool is_terminal_stmt = false;

    // Use stack to evaluate expressions
    std::stack<ValueInfo> cal_stack;

    struct Variable
    {
        Type *type;
        optional<string> id;
    };
    // pushed by ASTVarDeclaration, popped by consumer
    stack<Variable> var_decl_result;

    struct TypeSpecifierResult
    {
        Type *type;
    };
    // pushed by type specifiers, popped by consumer
    stack<TypeSpecifierResult> type_specifier_res;
    vector<optional<string>> latest_func_def_param_names;

    // Struct scope, for member functions
    vector<StructType *> struct_nest;
    std::string get_struct_id_prefix()
    {
        if (!struct_nest.empty())
            return struct_nest.back()->get_id() + ".";
        else
            return "";
    }

    // Function scope
    stack<Function *> function_nest;

    // ======================================================================

    /**
     * @brief Naming counters
     * 
     */
    size_t anonymous_struct_name_cnt = 0;
    size_t label_name_cnt = 0;

    // ======================================================================
    /**
     * @brief For constructors, assign, arithmatic operations, etc. 
     * 
     */

    void create_construct(string variable_name, Type *variable_type, ValueInfo init_value = {nullptr})
    {
        Value *address;
        if (scope.in_global())
        {
            // TODO Initialize global variables
            // Constant *init_val = ConstantZero::get(baseType, MOD);
            address = GlobalVariable::create(variable_name, MOD, variable_type, false, nullptr);
        }
        else
            address = builder->create_alloca(variable_type);

        scope.push(variable_name, address);

        // Initialization
        if (init_value.has_value())
        {
            if (variable_type->is_reference())
            {
                assert(init_value.value && init_value.is_lvalue);
                builder->create_store(init_value.value, address);
            }
            else // Default initialization
            {
                create_assign(address, init_value);
            }
        }
    }

    ValueInfo create_assign(Value *address, ValueInfo value)
    {
        assert(value.has_value());
        auto type = dynamic_cast<PointerType *>(address->get_type())->get_element_type();
        if (type->is_reference())
        {
            address = builder->create_load(address);
            create_assign(address, value);
            return ValueInfo{address, true};
        }
        else if (type->is_struct_type()) // Copy for structures
        {
            assert(value.is_lvalue && dynamic_cast<PointerType *>(value.value->get_type())->get_element_type()->is_struct_type());
            auto src_struct_type = dynamic_cast<StructType *>(value.value->get_type()->get_pointer_element_type());
            assert(type == src_struct_type);
            if (/*TODO*/ false)
            {
                return ValueInfo{address, true};
            }
            else // Default copy for structures
            {
                for (int i = 0; i < src_struct_type->get_members().size(); i++)
                {
                    auto dest_member_addr = builder->create_gep(address, {CONST_INT(0), CONST_INT(i)});
                    auto src_member_addr = builder->create_gep(value.value, {CONST_INT(0), CONST_INT(i)});
                    create_assign(dest_member_addr, {src_member_addr, true});
                }
                return ValueInfo{address, true};
            }
        }
        else // Default copy for scalar types
        {
            auto true_value = get_r_value(value);
            compulsiveTypeConvert(true_value, type);
            builder->create_store(true_value, address);
            return ValueInfo{address, true};
        }
    }

    ValueInfo create_call(ValueInfo operand, std::vector<ValueInfo> args)
    {
        if (operand.struct_member.is_member_func)
        {
            args.insert(args.begin(), ValueInfo{operand.struct_member.belonged_struct_ptr});
            return native_call(dynamic_cast<Function *>(operand.value), args);
        }
        else if (operand.get_type()->is_pointer_type() && operand.get_type()->get_pointer_element_type()->is_function_type())
        {
            auto func_ptr = get_r_value(operand);
            return native_call(func_ptr, args);
        }
        else // Default operation for calling
        {
            return native_call(operand.value, args);
        }
    }

    ValueInfo create_subscript(ValueInfo operand, ValueInfo subscript)
    {
        if (operand.get_type()->is_pointer_type()) // Default operation for pointers
        {
            return ValueInfo{builder->create_gep(get_r_value(operand), {get_r_value(subscript)}), true};
        }
        else if (operand.get_type()->is_array_type())
        {
            assert(operand.is_lvalue);
            return ValueInfo{builder->create_gep(operand.value, {CONST_INT(0), get_r_value(subscript)}), true};
        }
        else
        {
            throw "Operator overloading is not implemented"; // TODO
        }
    }

    ValueInfo create_member_access(ValueInfo operand, string member_id)
    {
        assert(operand.get_type()->is_struct_type());
        auto struct_type = dynamic_cast<StructType *>(operand.get_type());
        int member_idx = struct_type->get_element_index(member_id);
        if (member_idx < 0)
        {
            auto function = scope.find(struct_type->get_id() + "." + member_id);
            assert(dynamic_cast<Function *>(function) && "Undefined struct member");
            ValueInfo res;
            res.value = function;
            res.is_lvalue = false;
            res.struct_member.belonged_struct_ptr = operand.value;
            res.struct_member.is_member_func = true;
            return res;
        }
        else
        {
            auto member = builder->create_gep(operand.value, {CONST_INT(0), CONST_INT(member_idx)});
            return ValueInfo{member, true};
        }
    }

    ValueInfo create_dereference(ValueInfo operand)
    {
        if (operand.get_type()->is_pointer_type())
        {
            return ValueInfo{get_r_value(operand), true};
        }
        else
        {
            // TODO
            throw "Dereference overloading not implented";
        }
    }

    ValueInfo create_addressof(ValueInfo operand)
    {
        // if (operand.get_type()->is_function_type())
        // {
        //     operand.value->
        // }
        // else
        {
            assert(operand.is_lvalue);
            return ValueInfo{operand.value};
        }
    }

    ValueInfo create_mul(ValueInfo loperand, ValueInfo roperand, MulOp op)
    {
        char c_op = op == MulOp::OP_MUL ? '*' : '/';
        if (operator_overload_table.find({c_op, loperand.get_type()}) != operator_overload_table.end())
        {
            auto overloaded_func = operator_overload_table[{c_op, loperand.get_type()}];
            return native_call(overloaded_func, {create_addressof(loperand), roperand});
        }
        else
        {
            auto l = get_r_value(loperand);
            auto r = get_r_value(roperand);
            augmentTypeConvert(l, r);
            return ValueInfo{
                mulFuncTable[{loperand.get_type(), op}](l, r)};
        }
    }

    ValueInfo create_add(ValueInfo loperand, ValueInfo roperand, AddOp op)
    {
        char c_op = op == AddOp::OP_PLUS ? '+' : '-';
        if (operator_overload_table.find({c_op, loperand.get_type()}) != operator_overload_table.end())
        {
            auto overloaded_func = operator_overload_table[{c_op, loperand.get_type()}];
            return native_call(overloaded_func, {create_addressof(loperand), roperand});
        }
        else if (loperand.get_type()->is_pointer_type() && roperand.get_type()->is_integer_type())
        {
            return ValueInfo{builder->create_gep(get_r_value(loperand), {get_r_value(roperand)}), false};
        }
        else
        {
            auto l = get_r_value(loperand);
            auto r = get_r_value(roperand);
            augmentTypeConvert(l, r);
            return ValueInfo{
                addFuncTable[{loperand.get_type(), op}](l, r)};
        }
    }

    ValueInfo create_rel(ValueInfo loperand, ValueInfo roperand, RelOp op)
    {
        // TODO
        return ValueInfo{
            compFuncTable[{loperand.get_type(), op}](get_r_value(loperand), get_r_value(roperand))};
    }

    // ======================================================================
    /**
     * @brief Some helper functions
     * 
     */

    Value *get_r_value(ValueInfo value)
    {
        assert(!value.value->get_type()->is_struct_type());
        if (value.is_lvalue)
            return builder->create_load(value.value);
        else
            return value.value;
    }

    ValueInfo native_call(Value *callee, std::vector<ValueInfo> args)
    {
        std::vector<Value *> args_values;
        for (auto arg : args)
        {
            Value *arg_value = arg.value;
            if (!arg.get_type()->is_struct_type())
                arg_value = get_r_value(arg);
            args_values.push_back(arg_value);
        }
        Type *return_type;
        if (callee->get_type()->is_function_type())
            return_type = dynamic_cast<FunctionType *>(callee->get_type())->get_return_type();
        else
        {
            assert(callee->get_type()->is_pointer_type());
            assert(callee->get_type()->get_pointer_element_type()->is_function_type());
            auto func_type = dynamic_cast<FunctionType *>(callee->get_type()->get_pointer_element_type());
            return_type = func_type->get_return_type();
        }
        ValueInfo return_value;
        if (return_type->is_struct_type())
        {
            auto return_value_write_ptr = builder->create_alloca(return_type);
            builder->create_call(callee, std::move(args_values), return_value_write_ptr);
            return_value = ValueInfo{return_value_write_ptr, true};
        }
        // else if (return_type->is_reference())
        // {
        //     return_value = ValueInfo{builder->create_call(callee, std::move(args_values)), true};
        // }
        else
        {
            return_value = ValueInfo{builder->create_call(callee, std::move(args_values))};
        }
        return return_value;
    }

    // TODO Support `auto`
    // bool is_auto(ASTTypeSpecifier &node)
    // {
    //     auto named_type_specifier = dynamic_cast<ASTNamedType *>(&node);
    //     if (!named_type_specifier)
    //         return false;
    //     else
    //     {
    //         if (named_type_specifier->type_name == "auto")
    //             return true;
    //         else
    //             return false;
    //     }
    // }

    // ======================================================================
    /**
     * @brief Convert `enum CminusType` to `Type *`
     * @param CminusType a `enum CminusType` value
     * @return  a `Type *` value. 
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

    Value *getArrOrPtrAddr(Value *var, Value *index)
    {
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

    /**
     * @brief Compulsively convert value `origin` to `target` type.
     * 
     * e.g. If `a` is of type `int` and has a non-zero value, 
     * `compulsiveTypeConvert(a, GET_BOOL)` will change `a` to 
     * a bool type with value true. 
     * 
     */
    void compulsiveTypeConvert(Value *&origin, Type *target)
    {
        if (origin->get_type() != target && !(origin->get_type()->is_function_type() && target->get_pointer_element_type()->is_function_type()))
            compulsiveTypeConvertTable[{origin->get_type(), target}](origin);
    }

    /**
     * @brief Convert left/right to the same type
     * 
     * e.g. If left is of type bool, right is of type int, 
     * then left is converted to type int with value 1. 
     * 
     * The rank of types are as following: 
     * 1. float
     * 2. int
     * 3. bool
     * 
     * The conversion can only be performed from a lower-rank type
     * to a higher-rank type. 
     * 
     * @param left 
     * @param right 
     * @return Type* The type of left and right after conversion.
     */
    Type *augmentTypeConvert(Value *&left, Value *&right)
    {
        auto resType = std::max(left->get_type(), right->get_type(), [&](Type *l, Type *r)
                                { return typeOrderRank[l] < typeOrderRank[r]; });
        compulsiveTypeConvert(left, resType);
        compulsiveTypeConvert(right, resType);
        return resType;
    }

    /**
     * @brief Same as `Type *augmentTypeConvert(Value *&left, Value *&right)`
     * with the exception that, after conversion, the rank of
     * the type of left&right cannot be lower than min.
     * 
     * e.g. if left and right are both of type bool, 
     * after running `compulsiveTypeConvert(left, right, GET_INT)`,
     * both of them are converted to int. 
     * @return Type* 
     */
    Type *augmentTypeConvert(Value *&left, Value *&right, Type *min)
    {
        auto resType = std::max({left->get_type(), right->get_type(), min}, [&](Type *l, Type *r)
                                { return typeOrderRank[l] < typeOrderRank[r]; });
        compulsiveTypeConvert(left, resType);
        compulsiveTypeConvert(right, resType);
        return resType;
    }

    BasicBlock *new_basic_block()
    {
        return BasicBlock::create(MOD, std::to_string(label_name_cnt++), function_nest.top());
    }

    // Value *get_arr_elem_addr(IRBuilder *builder, Scope &scope, Value *arr, ASTExpression &offset)
    // {
    //     offset.accept(*this);
    //     auto index = cal_stack.top();
    //     cal_stack.pop();
    //     compulsiveTypeConvert(index, GET_INT32);

    //     // Examine wether the index is negative
    //     auto isIdxNeg = builder->create_icmp_lt(index, CONST_INT(0));
    //     auto negBB = newBasicBlock();
    //     auto posBB = newBasicBlock();
    //     builder->create_cond_br(isIdxNeg, negBB, posBB);
    //     builder->set_insert_point(negBB);
    //     auto negExcept = scope.find("neg_idx_except");
    //     builder->create_call(negExcept, {});
    //     builder->create_br(posBB);
    //     builder->set_insert_point(posBB);

    //     return getArrOrPtrAddr(arr, index);
    // }
};
#endif
