#include "cminusf_builder.hpp"

// use these macros to get constant value
#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) \
    ConstantZero::get(var_type, module.get())

/**
 * Return module;
 * The type is Module *
 */
#define MOD this->module.get()

#define GET_INT32 Type::get_int32_type(MOD)
#define GET_FLOAT Type::get_float_type(MOD)
#define GET_VOID Type::get_void_type(MOD)

/**
 * @brief Convert `enum CminusType` to `Type *`
 * @arg CminusType: a `enum CminusType` value
 * @arg res: a `Type *` value. The converted value will be assigned to it.
 * 
 */
#define CT2T(CminusType, res) \
    switch (CminusType)       \
    {                         \
    case TYPE_INT:            \
        res = GET_INT32;      \
        break;                \
    case TYPE_FLOAT:          \
        res = GET_FLOAT;      \
    case TYPE_VOID:           \
        res = GET_VOID;       \
    default:                  \
        break;                \
    }

#define INIT(Ty) ConstantZero::get((Ty), MOD)

// You can define global variables here
// to store state

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

void CminusfBuilder::visit(ASTProgram &node)
{
    this->scope.enter();
    for (auto &&declaration : node.declarations)
    {
        declaration->accept(*this);
    }
}

void CminusfBuilder::visit(ASTNum &node) {}

void CminusfBuilder::visit(ASTVarDeclaration &node)
{
    Type *baseType, *finalType;
    CT2T(node.type, baseType)
    if (node.num) // is an array
        finalType = ArrayType::get(baseType, node.num->i_val);
    else
        finalType = baseType;
    if (this->scope.in_global())
        this->scope.push(node.id, GlobalVariable::create(node.id, MOD, finalType, false, INIT(finalType)));
    else
        this->scope.push(node.id, this->builder->create_alloca(finalType));
}

void CminusfBuilder::visit(ASTFunDeclaration &node){}

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
