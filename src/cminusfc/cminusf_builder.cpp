#include "cminusf_builder.hpp"

using namespace std::placeholders;
using namespace std;

void CminusfBuilder::visit(ASTProgram &node)
{
    std::for_each(node.definitions.begin(), node.definitions.end(), [&](auto &ptr)
                  { ptr->accept(*this); });
}

/**
 * NOTE Result is pushed to var_decl_result
 */
void CminusfBuilder::visit(ASTVarDeclaration &node)
{
    cerr << "Enter ASTVarDeclaration\n";
    push(var_decl_result); // NOTE Push var_decl_result
    node.type_specifier->accept(*this);
    var_decl_result.top().type = pop(type_specifier_res).type;
    node.decl_expression->accept(*this);
}

void CminusfBuilder::visit(ASTDeclarationIdentifier &node)
{
    cerr << "Enter ASTDeclarationIdentifier\n";
    if (node.is_ref)
        var_decl_result.top().type = ReferenceType::get_reference_type(var_decl_result.top().type);
    var_decl_result.top().id = node.id;
}

void CminusfBuilder::visit(ASTDeclarationDereference &node)
{
    cerr << "Enter ASTDeclarationDereference\n";
    assert(var_decl_result.top().type->get_module());
    var_decl_result.top().type = PointerType::get(var_decl_result.top().type);
    node.expression->accept(*this);
}

void CminusfBuilder::visit(ASTDeclarationCall &node)
{
    cerr << "Enter ASTDeclarationCall\n";
    auto return_type = var_decl_result.top().type;
    std::vector<Type *> params_type;
    vector<optional<string>> param_names;
    for (auto &&param : node.params)
    {
        param->accept(*this);
        auto param_info = pop(var_decl_result); // NOTE var_decl_result consumed
        param_names.push_back(param_info.id);
        params_type.push_back(param_info.type);
    }
    latest_func_def_param_names = std::move(param_names);
    var_decl_result.top().type = FunctionType::get(return_type, params_type, MOD);
    node.callee->accept(*this);
}

void CminusfBuilder::visit(ASTDeclarationSubscript &node)
{
    cerr << "Enter ASTDeclarationSubscript\n";
    assert(node.subscript.has_value());
    auto element_type = var_decl_result.top().type;
    var_decl_result.top().type = ArrayType::get(element_type, node.subscript.value());
    node.array->accept(*this);
}

void CminusfBuilder::visit(ASTFunDefinition &node)
{
    cerr << "Enter ASTFunDefinition\n";
    node.var_declaration->accept(*this);
    auto function_prototype = pop(var_decl_result); // NOTE var_decl_result consumed
    assert(function_prototype.type->is_function_type());
    auto &params = dynamic_cast<FunctionType *>(function_prototype.type)->get_params();
    assert(all_of(params.begin(), params.end(), [&](Type *type)
                  { return !type->is_array_type(); }));
    assert(function_prototype.id.has_value());
    assert(all_of(latest_func_def_param_names.begin(), latest_func_def_param_names.end(), [&](auto id)
                  { return id.has_value(); }));
    if (!struct_nest.empty()) // Member functions
    {
        auto orig_func_ty = dynamic_cast<FunctionType *>(function_prototype.type);
        vector<Type *> params{PointerType::get(struct_nest.back())};
        for (auto &&param_ty : orig_func_ty->get_params())
            params.push_back(param_ty);
        function_prototype.type = FunctionType::get(orig_func_ty->get_return_type(), params, MOD);
        function_prototype.id = get_struct_id_prefix() + function_prototype.id.value();
        latest_func_def_param_names.insert(latest_func_def_param_names.begin(), "this");
    }
    Function *func = Function::create(dynamic_cast<FunctionType *>(function_prototype.type), function_prototype.id.value(), MOD);
    scope.push(function_prototype.id.value(), func);

    function_nest.push(func);

    compound_stmt_is_func_body = true;
    auto bb = BasicBlock::create(MOD, "entry", func);
    builder->set_insert_point(bb);

    node.function_body->accept(*this);

    if (!isTerminalStmt)
    {
        if (function_nest.top()->get_return_type() != GET_VOID)
            throw "Non void function without return statement is forbidden";
        else
        {
            isTerminalStmt = true;
            builder->create_void_ret();
        }
    }
    function_nest.pop();
    isTerminalStmt = false;
}

/**
 * NOTE NOT for member declarations in structures. 
 */
void CminusfBuilder::visit(ASTVarDefinition &node)
{
    cerr << "Enter ASTVarDefinition\n";

    node.var_declaration->accept(*this);
    auto var_declaration = pop(var_decl_result); // NOTE consumed

    ValueInfo init_value;
    if (node.init_value.has_value())
    {
        node.init_value.value()->accept(*this);
        init_value = pop(cal_stack);
    }

    // TODO Support `auto`

    if (var_declaration.id.has_value())
        create_construct(var_declaration.id.value(), var_declaration.type, init_value);
}

/**
 * @brief Push to type_specifier_res
 */
void CminusfBuilder::visit(ASTStructSpecification &node)
{
    cerr << "Enter ASTStructSpecification\n";
    auto struct_id = node.struct_id.value_or("anonymous_struct." + to_string(anonymous_struct_name_cnt++));
    vector<ASTVarDefinition *> var_members_def;
    vector<ASTFunDefinition *> fun_members_def;
    for (auto def : node.definitions)
    {
        if (dynamic_cast<ASTVarDefinition *>(def.get()))
        {
            var_members_def.push_back(dynamic_cast<ASTVarDefinition *>(def.get()));
        }
        else
        {
            assert(dynamic_cast<ASTFunDefinition *>(def.get()));
            fun_members_def.push_back(dynamic_cast<ASTFunDefinition *>(def.get()));
        }
    }
    vector<StructType::StructMember> var_members;
    for (auto var_member_def : var_members_def)
    {
        var_member_def->var_declaration->accept(*this);
        auto var_member_decl = pop(var_decl_result);
        assert(var_member_decl.id.has_value());
        var_members.push_back({var_member_decl.type, var_member_decl.id.value()});
        // TODO In-structure initialization
    }

    MOD->add_struct(struct_id, std::move(var_members));
    auto struct_type = StructType::get(struct_id, MOD);

    struct_nest.push_back(struct_type);
    for (auto func_member_def : fun_members_def)
        func_member_def->accept(*this);
    struct_nest.pop_back();

    type_specifier_res.push({struct_type});
}

/**
 * @brief Push to type_specifier_res
 */
void CminusfBuilder::visit(ASTNamedType &node)
{
    cerr << "Enter ASTNamedType\n";
    push(type_specifier_res);
    if (node.type_name == "int")
    {
        type_specifier_res.top().type = GET_INT32;
    }
    else if (node.type_name == "float")
    {
        type_specifier_res.top().type = GET_FLOAT;
    }
    else if (node.type_name == "auto")
    {
        throw "`auto` is not implemented";
    }
    else
    {
        type_specifier_res.top().type = module->get_struct_type(node.type_name);
    }
}

void CminusfBuilder::visit(ASTCompoundStmt &node)
{
    cerr << "Enter ASTCompoundStmt\n";
    scope.enter();
    bool this_is_func_body = compound_stmt_is_func_body;
    if (compound_stmt_is_func_body) // This compoundStmt is a function body
    {
        auto param_name_iter = latest_func_def_param_names.begin();
        auto param_type_iter = function_nest.top()->get_function_type()->get_params().begin();
        auto arg_iter = function_nest.top()->get_args().begin();
        for (; arg_iter != function_nest.top()->get_args().end(); arg_iter++, param_name_iter++, param_type_iter++)
            create_construct(param_name_iter->value(), *param_type_iter, {*arg_iter, (*param_type_iter)->is_struct_type()});
        compound_stmt_is_func_body = false;
    }
    for (auto &&stmt : node.statement_list)
    {
        stmt->accept(*this);
        if (isTerminalStmt)
            break;
    }
    scope.exit();
}

void CminusfBuilder::visit(ASTExpressionStmt &node)
{
    cerr << "Enter ASTExpressionStmt\n";
    if (node.expression.has_value())
        node.expression.value()->accept(*this);
}

void CminusfBuilder::visit(ASTSelectionStmt &node)
{
    cerr << "Enter ASTSelectionStmt\n";
}

void CminusfBuilder::visit(ASTIterationStmt &node)
{
    cerr << "Enter ASTIterationStmt\n";
}

void CminusfBuilder::visit(ASTReturnStmt &node)
{
    cerr << "Enter ASTReturnStmt\n";
    if (node.expression.has_value())
    {
        node.expression.value()->accept(*this);
        auto ret_value = pop(cal_stack);
        if (function_nest.top()->get_return_type()->is_struct_type())
        {
            assert(ret_value.has_value() &&
                   ret_value.value->get_type()->is_pointer_type() &&
                   dynamic_cast<PointerType *>(ret_value.value->get_type())->get_element_type()->is_struct_type() &&
                   ret_value.is_lvalue);
            create_assign(function_nest.top()->get_ret(), ret_value);
            builder->create_void_ret();
        }
        else
            builder->create_ret(get_r_value(ret_value));
    }
    else
        builder->create_void_ret();
    isTerminalStmt = true;
}

void CminusfBuilder::visit(ASTVar &node)
{
    cerr << "Enter ASTVar\n";
    auto var = scope.find(node.var_id);
    if (var->get_type()->is_function_type())
    {
        cal_stack.push({var, false});
    }
    else if (var->get_type()->get_pointer_element_type()->is_array_type()) // is of array type
    {
        auto firstElemAddr = builder->create_gep(var, {CONST_INT(0), CONST_INT(0)});
        cal_stack.push({firstElemAddr, false});
    }
    else if (var->get_type()->get_pointer_element_type()->is_reference())
    {
        cal_stack.push({builder->create_load(var), true});
    }
    else
        cal_stack.push({var, true});
}

void CminusfBuilder::visit(ASTNum &node)
{
    cerr << "Enter ASTNum\n";
    Value *num;
    if (node.num_type == TYPE_INT)
        num = CONST_INT(node.num.i);
    else
        num = CONST_FP(node.num.f);
    cal_stack.push(ValueInfo{num});
}

void CminusfBuilder::visit(ASTCall &node)
{
    cerr << "Enter ASTCall\n";
    node.callee->accept(*this);
    auto callee = pop(cal_stack);
    std::vector<ValueInfo> args;
    for (auto arg : node.args)
    {
        arg->accept(*this);
        args.push_back(pop(cal_stack));
    }

    cal_stack.push(create_call(callee, std::move(args)));
}

void CminusfBuilder::visit(ASTSubscript &node)
{
    cerr << "Enter ASTSubscript\n";
    node.array->accept(*this);
    auto array = pop(cal_stack);
    node.subscript->accept(*this);
    auto subscript = pop(cal_stack);
    cal_stack.push(create_subscript(array, subscript));
}

void CminusfBuilder::visit(ASTMemberAccess &node)
{
    cerr << "Enter ASTMemberAccess\n";
    node.object->accept(*this);
    auto object = pop(cal_stack);
    cal_stack.push(create_member_access(object, node.member_id));
}

void CminusfBuilder::visit(ASTUnaryAddExpression &node)
{
    cerr << "Enter ASTUnaryAddExpression\n";
    // TODO
    throw "Unary additive operators are not implemented yet";
}

void CminusfBuilder::visit(ASTDereference &node)
{
    cerr << "Enter ASTDereference\n";
    node.expression->accept(*this);
    auto operand = pop(cal_stack);
    cal_stack.push(create_dereference(operand));
}

void CminusfBuilder::visit(ASTAddressof &node)
{
    cerr << "Enter ASTAddressof\n";
    node.expression->accept(*this);
    auto operand = pop(cal_stack);
    cal_stack.push(create_addressof(operand));
}

void CminusfBuilder::visit(ASTMultiplicativeExpression &node)
{
    cerr << "Enter ASTMultiplicativeExpression\n";
    node.l_expression->accept(*this);
    auto loperand = pop(cal_stack);
    node.r_expression->accept(*this);
    auto roperand = pop(cal_stack);
    cal_stack.push(create_mul(loperand, roperand, node.op));
}

void CminusfBuilder::visit(ASTAdditiveExpression &node)
{
    cerr << "Enter ASTAdditiveExpression\n";
    node.l_expression->accept(*this);
    auto loperand = pop(cal_stack);
    node.r_expression->accept(*this);
    auto roperand = pop(cal_stack);
    cal_stack.push(create_add(loperand, roperand, node.op));
}

void CminusfBuilder::visit(ASTRelationalExpression &node)
{
    cerr << "Enter ASTRelationalExpression\n";
    node.l_expression->accept(*this);
    auto loperand = pop(cal_stack);
    node.r_expression->accept(*this);
    auto roperand = pop(cal_stack);
    cal_stack.push(create_rel(loperand, roperand, node.op));
}

void CminusfBuilder::visit(ASTAssignExpression &node)
{
    cerr << "Enter ASTAssignExpression\n";
    node.l_expression->accept(*this);
    auto loperand = pop(cal_stack);
    node.r_expression->accept(*this);
    auto roperand = pop(cal_stack);
    assert(loperand.is_lvalue);
    cal_stack.push(create_assign(loperand.value, roperand));
}
