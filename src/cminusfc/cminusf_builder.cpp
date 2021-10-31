#include "cminusf_builder.hpp"
#include "logging.hpp"

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
    baseType = CminusTypeConvertor(node.type);
    if (node.num) // is an array
    {
        LOG(INFO) << "is array";
        finalType = ArrayType::get(baseType, node.num->i_val);
    }
    else
        finalType = baseType;

    if (this->scope.in_global())
    {
        Constant *init_val = ConstantZero::get(baseType, MOD);
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
    if (!terminateStmt)
    {
        LOG_INFO << "No terminate stmt";
        if(lastEnteredFun->get_return_type()!=GET_VOID)
            LOG_ERROR << "Non void function without return statement is forbidden";
        else
            builder->create_void_ret();
    }
    terminateStmt = false;

    LOG(INFO) << "Exit func decl" << node.id;
}

void CminusfBuilder::visit(ASTParam &node)
{
    // TODO Refomat
    Type *ty;
    if (node.isarray)
        ty = PointerType::get(CminusTypeConvertor(node.type));
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
        stmt->accept(*this);
        if (terminateStmt)
            break;
    }
    scope.exit();
}

void CminusfBuilder::visit(ASTExpressionStmt &node)
{
    node.expression->accept(*this);
    cal_stack.pop();
}

void CminusfBuilder::visit(ASTSelectionStmt &node)
{
    auto exitBB = newBasicBlock();
    auto trueBB = newBasicBlock();
    auto falseBB = node.else_statement ? newBasicBlock() : exitBB;
    node.expression->accept(*this);
    auto cond_res = cal_stack.top();
    cal_stack.pop();
    compulsiveTypeConvert(cond_res, GET_BOOL);
    builder->create_cond_br(cond_res, trueBB, falseBB);
    builder->set_insert_point(trueBB);
    node.if_statement->accept(*this);
    auto if_stmt_terminate = terminateStmt;
    if (!terminateStmt)
        builder->create_br(exitBB);
    terminateStmt = false;
    if (node.else_statement)
    {
        builder->set_insert_point(falseBB);
        node.else_statement->accept(*this);
        if (!terminateStmt)
            builder->create_br(exitBB);
        terminateStmt = terminateStmt && if_stmt_terminate;
    }
    if (!terminateStmt)
        builder->set_insert_point(exitBB);
    else
        lastEnteredFun->remove(exitBB);
}

void CminusfBuilder::visit(ASTIterationStmt &node)
{
    auto condBB = newBasicBlock();
    auto loopBodyBB = newBasicBlock();
    auto exitBB = newBasicBlock();
    builder->create_br(condBB);
    builder->set_insert_point(condBB);
    node.expression->accept(*this);
    auto cond_res = cal_stack.top();
    cal_stack.pop();
    compulsiveTypeConvert(cond_res, GET_BOOL);
    builder->create_cond_br(cond_res, loopBodyBB, exitBB);
    builder->set_insert_point(loopBodyBB);
    node.statement->accept(*this);
    if (terminateStmt)
        terminateStmt = false;
    else
        builder->create_br(condBB);

    builder->set_insert_point(exitBB);
}

void CminusfBuilder::visit(ASTReturnStmt &node)
{
    if (node.expression)
    {
        node.expression->accept(*this);
        auto retVal = cal_stack.top();
        compulsiveTypeConvert(retVal, lastEnteredFun->get_return_type());
        builder->create_ret(retVal);
        cal_stack.pop();
    }
    else
        builder->create_void_ret();
    terminateStmt = true;
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
        auto index = cal_stack.top();
        compulsiveTypeConvert(index, GET_INT32);

        // Examine wether the index is negative
        auto isIdxNeg = builder->create_icmp_lt(index, CONST_INT(0));
        auto negBB = newBasicBlock();
        auto posBB = newBasicBlock();
        builder->create_cond_br(isIdxNeg, negBB, posBB);
        builder->set_insert_point(negBB);
        auto negExcept = scope.find("neg_idx_except");
        builder->create_call(negExcept, {});
        builder->create_br(posBB);
        builder->set_insert_point(posBB);

        obj_addr = getArrOrPtrAddr(var, index);

        cal_stack.pop();
        cal_stack.push(builder->create_load(obj_addr));
    }
    else if (var->get_type()->get_pointer_element_type()->is_array_type()) // is of array type
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
        auto index = cal_stack.top();
        cal_stack.pop();
        obj_addr = getArrOrPtrAddr(var, index);
    }
    else
        obj_addr = var;
    //=========================================
    auto val = cal_stack.top();
    cal_stack.pop();
    compulsiveTypeConvert(val, obj_addr->get_type()->get_pointer_element_type());
    builder->create_store(val, obj_addr);
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

    auto resType = augmentTypeConvert(left, right, GET_INT32);
    Value *res = compFuncTable[{resType, node.op}](left, right);
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

    auto resType = augmentTypeConvert(left, right, GET_INT32);
    Value *res = addFuncTable[{resType, node.op}](left, right);
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

    auto resType = augmentTypeConvert(left, right, GET_INT32);
    Value *res = mulFuncTable[{resType, node.op}](left, right);
    cal_stack.push(res);
}

void CminusfBuilder::visit(ASTCall &node)
{
    LOG_INFO << "Enter call " << node.id;
    auto callee = scope.find(node.id);
    std::vector<Value *> args;
    auto formalArg = dynamic_cast<Function *>(callee)->arg_begin();
    for (auto &&expr : node.args)
    {
        expr->accept(*this);
        auto arg = cal_stack.top();
        cal_stack.pop();
        compulsiveTypeConvert(arg, (*formalArg++)->get_type());
        args.push_back(arg);
    }
    cal_stack.push(builder->create_call(callee, std::move(args)));
    LOG_INFO << "Exit call" << node.id;
}
