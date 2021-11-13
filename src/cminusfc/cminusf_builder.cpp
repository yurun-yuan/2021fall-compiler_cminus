#include "cminusf_builder.hpp"

/**
 * Preambles can be defined here, including `using` 
 * statements and `#include`.
 * 
 */

#include "logging.hpp"
#include <stack>
#include <utility>
#include <functional>
#include <algorithm>

using namespace std::placeholders;
using ConvertorFuncType = std::function<void(Value *&)>;
using CompFuncType = std::function<Value *(Value *left, Value *right)>;
using AddFuncType = std::function<Value *(Value *left, Value *right)>;
using MulFuncType = std::function<Value *(Value *left, Value *right)>;

/**
 * Some infrastructures can be defined here. 
 * 
 * If you want to tell other members how to use them, 
 * write comments, or specify the usage in 
 * Reports/3-ir-gen/Group discussions.md part "自建基础设施文档"
 * 
 */

// use these macros to get constant value
#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) \
    ConstantZero::get(var_type, module.get())

#define MOD builder->get_module()

// Macros for Type
#define GET_INT32 Type::get_int32_type(MOD)
#define GET_FLOAT Type::get_float_type(MOD)
#define GET_VOID Type::get_void_type(MOD)
#define GET_BOOL Type::get_int1_type(MOD)

/**
 * @brief Convert `enum CminusType` to `Type *`
 * @param builder just pass the builder in CminusfBuilder to it
 * @param CminusType a `enum CminusType` value
 * @return  a `Type *` value. 
 * 
 */
Type *CminusTypeConvertor(IRBuilder *builder, enum CminusType c)
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

#define INIT(Ty) ConstantZero::get((Ty), MOD)

// See `group discussion.md` for more info
std::map<Type *, int> typeOrderRank;
std::map<std::pair<Type *, enum RelOp>, CompFuncType> compFuncTable;
std::map<std::pair<Type *, enum AddOp>, AddFuncType> addFuncTable;
std::map<std::pair<Type *, enum MulOp>, MulFuncType> mulFuncTable;
std::map<std::pair<Type *, Type *>, ConvertorFuncType> compulsiveTypeConvertTable;

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
    if (origin->get_type() != target)
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

/**
 * Main task: write `visit` functions
 * 
 */

void CminusfBuilder::visit(ASTProgram &node)
{
    /**
     * Since I cannot modify the constructor defined in 
     * `.hpp` file, I initiate some variables here. 
     * 
     */
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

    std::for_each(node.declarations.begin(), node.declarations.end(), [&](auto &ptr)
                  { ptr->accept(*this); });
}

void CminusfBuilder::visit(ASTNum &node) {}

void CminusfBuilder::visit(ASTVarDeclaration &node)
{}

void CminusfBuilder::visit(ASTFunDeclaration &node) {}

void CminusfBuilder::visit(ASTParam &node) {}

void CminusfBuilder::visit(ASTCompoundStmt &node) {}

void CminusfBuilder::visit(ASTExpressionStmt &node) {}

void CminusfBuilder::visit(ASTSelectionStmt &node) {}

void CminusfBuilder::visit(ASTIterationStmt &node) {}

void CminusfBuilder::visit(ASTReturnStmt &node) {}

void CminusfBuilder::visit(ASTVar &node) {}

void CminusfBuilder::visit(ASTAssignExpression &node) {}

void CminusfBuilder::visit(ASTSimpleExpression &node) {}

void CminusfBuilder::visit(ASTAdditiveExpression &node) {}

void CminusfBuilder::visit(ASTTerm &node) {}

void CminusfBuilder::visit(ASTCall &node) {}
