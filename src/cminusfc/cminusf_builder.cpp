#include "cminusf_builder.hpp"
#include "logging.hpp"

// use these macros to get constant value
#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) \
    ConstantZero::get(var_type, module.get())

#define CONST_INT(num) \
    ConstantInt::get(num, MOD)
#define CONST_FP(num) \
    ConstantFP::get(num, MOD)

#define GET_CONST(astNum) \
    (astNum).type == TYPE_INT ? (Value *)CONST_INT((astNum).i_val) : (Value *)CONST_FP((astNum).f_val)

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

void CminusfBuilder::visit(ASTProgram &node)
{
    LOG(INFO) << "Enter program";
    for (auto &&declaration : node.declarations)
    {
        declaration->accept(*this);
    }
}

void CminusfBuilder::visit(ASTNum &node)
{
    LOG(INFO) << "Enter num " << node.i_val << "; " << node.f_val;
    cal_stack.push(GET_CONST(node));
    LOG_INFO << "Exit num";
}

void CminusfBuilder::visit(ASTVarDeclaration &node)
{
    LOG(INFO) << "Enter var decl " << node.id;
    Type *baseType, *finalType;
    Constant *init_val = nullptr;
    baseType = CminusTypeConvertor(node.type);
    if (node.num) // is an array
    {
        LOG(INFO) << "is array";
        finalType = ArrayType::get(baseType, node.num->i_val);
        init_val = ConstantZero::get(baseType, MOD);
    }
    else
        finalType = baseType;

    if (this->scope.in_global())
    {
        auto val = GlobalVariable::create(node.id, MOD, finalType, false, init_val);
        this->scope.push(node.id, val);
    }
    else
        this->scope.push(node.id, this->builder->create_alloca(finalType));
    LOG(INFO) << "Exit var decl" << node.id;
}

void CminusfBuilder::visit(ASTFunDeclaration &node)
{
    LOG(INFO) << "Enter fun decl " << node.id << "";
    std::vector<Type *> Args{};
    for (auto &&param : node.params)
    {
        // TODO reformat
        Type *ty;
        if (param->isarray)
        {
            if (param->type == TYPE_INT)
                ty = Type::get_int32_ptr_type(MOD);
            else
                ty = Type::get_float_ptr_type(MOD);
        }
        else
            ty = CminusTypeConvertor(param->type);
        Args.push_back(ty);
    }
    Type *returnType = CminusTypeConvertor(node.type);
    auto func = Function::create(FunctionType::get(returnType, Args), node.id, MOD);
    scope.push(node.id, func);
    lastEnteredFun = func;
    enteredFun = true;
    auto bb = BasicBlock::create(MOD, "entry", func);
    builder->set_insert_point(bb);
    params = &node.params;
    node.compound_stmt->accept(*this);

    LOG(INFO) << "Exit func decl" << node.id;
}

void CminusfBuilder::visit(ASTParam &node)
{
    // TODO Refomat
    Type *ty;
    if (node.isarray)
    {
        if (node.type == TYPE_INT)
            ty = Type::get_int32_ptr_type(MOD);
        else if (node.type == TYPE_FLOAT)
            ty = Type::get_float_ptr_type(MOD);
    }
    else
        ty = CminusTypeConvertor(node.type);
    auto alloca = builder->create_alloca(ty);
    scope.push(node.id, alloca);
    builder->create_store(*curArg++, alloca);
}

void CminusfBuilder::visit(ASTCompoundStmt &node)
{
    LOG(INFO) << "Enter compoundstmt";
    scope.enter();
    if (enteredFun) // This compoundStmt is a function body
    {
        curArg = lastEnteredFun->arg_begin();
        for (auto &&param : *params)
            param->accept(*this);
        params = nullptr;
        enteredFun = false;
    }
    for (auto &&varDecl : node.local_declarations)
        varDecl->accept(*this);
    for (auto &&stmt : node.statement_list)
    {
        if (isCtrlStmt(stmt.get()))
        {
            auto CtrlStmtExit = newBasicBlock();
            bb_stack.push(CtrlStmtExit);
            stmt->accept(*this);
            builder->set_insert_point(CtrlStmtExit);
        }
        else
            stmt->accept(*this);
    }
}

void CminusfBuilder::visit(ASTExpressionStmt &node)
{
    node.expression->accept(*this);
    cal_stack.pop();
}

void CminusfBuilder::visit(ASTSelectionStmt &node)
{
    auto exitBB = bb_stack.top();
    bb_stack.pop();
    auto trueBB = newBasicBlock();
    auto falseBB = node.else_statement ? newBasicBlock() : exitBB;
    node.expression->accept(*this);
    builder->create_cond_br(cal_stack.top(), trueBB, falseBB);
    cal_stack.pop();
    builder->set_insert_point(trueBB);
    node.if_statement->accept(*this);
    builder->create_br(exitBB);
    if (node.else_statement)
    {
        builder->set_insert_point(falseBB);
        node.else_statement->accept(*this);
        builder->create_br(exitBB);
    }
}

void CminusfBuilder::visit(ASTIterationStmt &node)
{
    auto exitBB = bb_stack.top();
    bb_stack.pop();
    auto condBB = newBasicBlock();
    auto loopBodyBB = newBasicBlock();
    builder->create_br(condBB);
    builder->set_insert_point(condBB);
    node.expression->accept(*this);
    builder->create_cond_br(cal_stack.top(), loopBodyBB, exitBB);
    cal_stack.pop();
    builder->set_insert_point(loopBodyBB);
    node.statement->accept(*this);
    builder->create_br(condBB);
}

void CminusfBuilder::visit(ASTReturnStmt &node)
{
    if (node.expression)
    {
        node.expression->accept(*this);
        builder->create_ret(cal_stack.top());
        cal_stack.pop();
    }
    else
        builder->create_void_ret();
}

void CminusfBuilder::visit(ASTVar &node)
{
    // TODO Can be extracted as a function
    // ========================================
    auto var = scope.find(node.id);
    Value *obj_addr;
    if (node.expression) // is an element of an array
    {
        node.expression->accept(*this);
        obj_addr = builder->create_gep(var, {CONST_INT(0), cal_stack.top()});
        cal_stack.pop();
        cal_stack.push(builder->create_load(obj_addr));
    }
    else if (dynamic_cast<AllocaInst *>(var) &&
             dynamic_cast<AllocaInst *>(var)->get_alloca_type()->is_array_type()) // is of array type
    {
        auto firstElemAddr = builder->create_gep(var, {CONST_INT(0), CONST_INT(0)});
        cal_stack.push(firstElemAddr);
    }
    else
    {
        obj_addr = var;
        cal_stack.push(builder->create_load(obj_addr));
    }
    //=========================================
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
    builder->create_store(cal_stack.top(), obj_addr);
    cal_stack.pop();
    cal_stack.push(builder->create_load(obj_addr));
}

void CminusfBuilder::visit(ASTSimpleExpression &node)
{
    if (!node.additive_expression_r)
    {
        node.additive_expression_l->accept(*this);
        return;
    }
    node.additive_expression_l->accept(*this);
    node.additive_expression_r->accept(*this);
    auto right = cal_stack.top();
    cal_stack.pop();
    auto left = cal_stack.top();
    cal_stack.pop();
    Value *res;
    Type *resType;

    // TODO extract as a function
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

void CminusfBuilder::visit(ASTAdditiveExpression &node)
{
    if (!node.additive_expression)
    {
        node.term->accept(*this);
        return;
    }
    node.additive_expression->accept(*this);
    node.term->accept(*this);
    auto right = cal_stack.top();
    cal_stack.pop();
    auto left = cal_stack.top();
    cal_stack.pop();

    Type *resType;
    Value *res;

    // TODO extract as a function
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

    if (resType == GET_INT32)
    {
        switch (node.op)
        {
        case OP_PLUS:
            res = builder->create_iadd(left, right);
            break;
        case OP_MINUS:
            res = builder->create_isub(left, right);
            break;
        default:
            break;
        }
    }
    else
    {
        switch (node.op)
        {
        case OP_PLUS:
            res = builder->create_fadd(left, right);
            break;
        case OP_MINUS:
            res = builder->create_fsub(left, right);
            break;
        default:
            break;
        }
    }
    cal_stack.push(res);
}

void CminusfBuilder::visit(ASTTerm &node)
{
    if (!node.term)
    {
        node.factor->accept(*this);
        return;
    }
    node.term->accept(*this);
    node.factor->accept(*this);
    auto right = cal_stack.top();
    cal_stack.pop();
    auto left = cal_stack.top();
    cal_stack.pop();

    Type *resType;
    Value *res;
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
    if (resType == GET_INT32)
    {
        switch (node.op)
        {
        case OP_MUL:
            res = builder->create_imul(left, right);
            break;
        case OP_DIV:
            res = builder->create_isdiv(left, right);
            break;
        default:
            break;
        }
    }
    else
    {
        switch (node.op)
        {
        case OP_MUL:
            res = builder->create_fmul(left, right);
            break;
        case OP_DIV:
            res = builder->create_fdiv(left, right);
            break;
        default:
            break;
        }
    }
    cal_stack.push(res);
}

void CminusfBuilder::visit(ASTCall &node)
{
    LOG_INFO << "Enter call " << node.id;
    std::vector<Value *> args;
    for (auto &&expr : node.args)
    {
        expr->accept(*this);
        auto arg = cal_stack.top();
        cal_stack.pop();
        args.push_back(arg);
    }
    builder->create_call(scope.find(node.id), std::move(args));
    LOG_INFO << "Exit call" << node.id;
}
