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

#define CONST_INT(num) \
    ConstantInt::get(num, MOD)
#define CONST_FP(num) \
    ConstantFP::get(num, MOD)

#define GET_CONST(astNum) \
    (astNum).type == TYPE_INT ? (Value *)CONST_INT((astNum).i_val) : (Value *)CONST_FP((astNum).f_val)

// TODO Render this a member function
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

void CminusfBuilder::visit(ASTNum &node)
{
    cal_stack.push(GET_CONST(node));
}

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

void CminusfBuilder::visit(ASTFunDeclaration &node) {}

void CminusfBuilder::visit(ASTParam &node) {}

void CminusfBuilder::visit(ASTCompoundStmt &node) {}

void CminusfBuilder::visit(ASTExpressionStmt &node)
{
    node.expression->accept(*this);
    cal_stack.pop();
}

void CminusfBuilder::visit(ASTSelectionStmt &node) {}

void CminusfBuilder::visit(ASTIterationStmt &node) {}

void CminusfBuilder::visit(ASTReturnStmt &node) {}

void CminusfBuilder::visit(ASTVar &node)
{
    // TODO Can be extracted as a function
    // ========================================
    auto var = scope.find(node.id);
    Value *obj_addr;
    if (node.expression) // is an array
    {
        node.expression->accept(*this);
        obj_addr = builder->create_gep(var, {CONST_INT(0), cal_stack.top()});
        cal_stack.pop();
    }
    else
        obj_addr = var;
    //=========================================
    cal_stack.push(builder->create_load(obj_addr));
}

void CminusfBuilder::visit(ASTAssignExpression &node)
{
    node.expression->accept(*this);
    // TODO Can be extracted as a function
    // ========================================
    auto var = scope.find(node.var->id);
    Value *obj_addr;
    if (node.var->expression) // is an array
    {
        node.var->expression->accept(*this);
        obj_addr = builder->create_gep(var, {CONST_INT(0), cal_stack.top()});
        cal_stack.pop();
    }
    else
        obj_addr = var;
    //=========================================
    cal_stack.pop();
    builder->create_store(cal_stack.top(), obj_addr);
    cal_stack.push(builder->create_load(obj_addr));
}

void CminusfBuilder::visit(ASTSimpleExpression &node)
{
    node.additive_expression_l->accept(*this);
    node.additive_expression_r->accept(*this);
    auto right = cal_stack.top();
    cal_stack.pop();
    auto left = cal_stack.top();
    cal_stack.pop();
    Value *res;
    Type *resType;
    if (left->get_type() == right->get_type())
        resType = left->get_type();
    else
    {
        if (left->get_type() == GET_INT32)
            left = builder->create_sitofp(left, GET_FLOAT);
        else
            right = builder->create_sitofp(right, GET_FLOAT);
        resType = GET_FLOAT;
    }

    // TODO Use function pointer to member functions 
    // to avoid the duplication
    if (resType == GET_INT32)
    {
        switch (node.op)
        {
        case OP_LT:
            res = builder->create_icmp_lt(left, right);
            break;
        case OP_LE:
            res = builder->create_icmp_le(left, right);
            break;
        case OP_EQ:
            res = builder->create_icmp_eq(left, right);
            break;
        case OP_NEQ:
            res = builder->create_icmp_ne(left, right);
            break;
        case OP_GT:
            res = builder->create_icmp_gt(left, right);
            break;
        case OP_GE:
            res = builder->create_icmp_ge(left, right);
            break;
        default:
            break;
        }
    }
    else
    {
        switch (node.op)
        {
        case OP_LT:
            res = builder->create_fcmp_lt(left, right);
            break;
        case OP_LE:
            res = builder->create_fcmp_le(left, right);
            break;
        case OP_EQ:
            res = builder->create_fcmp_eq(left, right);
            break;
        case OP_NEQ:
            res = builder->create_fcmp_ne(left, right);
            break;
        case OP_GT:
            res = builder->create_fcmp_gt(left, right);
            break;
        case OP_GE:
            res = builder->create_fcmp_ge(left, right);
            break;
        default:
            break;
        }
    }
    cal_stack.push(res);
}

void CminusfBuilder::visit(ASTAdditiveExpression &node) {}

void CminusfBuilder::visit(ASTTerm &node) {}

void CminusfBuilder::visit(ASTCall &node) {}
