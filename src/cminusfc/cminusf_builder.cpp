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
    push(var_decl_result); // NOTE Push var_decl_result
    node.type_specifier->accept(*this);
    var_decl_result.top().type = pop(type_specifier_res).type;
    node.decl_expression->accept(*this);
}

void CminusfBuilder::visit(ASTDeclarationIdentifier &node)
{
    if (node.is_ref)
        var_decl_result.top().type = ReferenceType::get_reference_type(var_decl_result.top().type);
    var_decl_result.top().id = node.id;
}

void CminusfBuilder::visit(ASTDeclarationDereference &node)
{
    assert(var_decl_result.top().type->get_module());
    var_decl_result.top().type = PointerType::get(var_decl_result.top().type);
    node.expression->accept(*this);
}

void CminusfBuilder::visit(ASTDeclarationCall &node)
{
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
    var_decl_result.top().type = FunctionType::get(return_type, params_type, MOD);
    var_decl_result.top().func_param_name = param_names;
    node.callee->accept(*this);
}

void CminusfBuilder::visit(ASTDeclarationSubscript &node)
{
    assert(node.subscript.has_value());
    auto element_type = var_decl_result.top().type;
    var_decl_result.top().type = ArrayType::get(element_type, node.subscript.value());
    node.array->accept(*this);
}

void CminusfBuilder::visit(ASTFunDefinition &node)
{
    node.var_declaration->accept(*this);
    auto& function_prototype = var_decl_result.top(); // NOTE var_decl_result consumed
    assert(function_prototype.type->is_function_type());
    auto &params = dynamic_cast<FunctionType *>(function_prototype.type)->get_params();
    assert(all_of(params.begin(), params.end(), [&](Type *type)
                  { return !type->is_array_type(); }));
    assert(function_prototype.id.has_value());
    auto orig_name = function_prototype.id.value();
    auto &latest_func_def_param_names = function_prototype.func_param_name.value();
    assert(all_of(latest_func_def_param_names.begin(), latest_func_def_param_names.end(), [&](auto id)
                  { return id.has_value(); }));
    bool is_operator_overload = false;
    if (!struct_nest.empty()) // Member functions
    {
        if (!struct_nest.empty() &&
            orig_name.size() == string("operator").size() + 1 &&
            orig_name.starts_with("operator") &&
            (orig_name.back() == '+' ||
             orig_name.back() == '-' ||
             orig_name.back() == '*' ||
             orig_name.back() == '/'))
            is_operator_overload = true;
        auto orig_func_ty = dynamic_cast<FunctionType *>(function_prototype.type);
        vector<Type *> params{PointerType::get(struct_nest.back())};
        for (auto &&param_ty : orig_func_ty->get_params())
            params.push_back(param_ty);
        function_prototype.type = FunctionType::get(orig_func_ty->get_return_type(), params, MOD);
        function_prototype.id = get_struct_id_prefix() + function_prototype.id.value();
        latest_func_def_param_names.insert(latest_func_def_param_names.begin(), "this");
    }
    if (is_operator_overload)
    {
        function_prototype.id.value().pop_back();
        function_prototype.id.value() += '.';
        if (orig_name.back() == '+')
            function_prototype.id.value() += "plus";
        else if (orig_name.back() == '-')
            function_prototype.id.value() += "minus";
        else if (orig_name.back() == '*')
            function_prototype.id.value() += "multiply";
        else if (orig_name.back() == '/')
            function_prototype.id.value() += "divide";
    }
    Function *func = Function::create(dynamic_cast<FunctionType *>(function_prototype.type), function_prototype.id.value(), MOD);
    scope.push(function_prototype.id.value(), func);

    if (is_operator_overload)
        // Operator overloading
        operator_overload_table[{orig_name.back(), struct_nest.back()}] = func;

    function_nest.push(func);

    compound_stmt_is_func_body = true;
    auto bb = BasicBlock::create(MOD, "entry", func);
    auto old_bb = builder->get_insert_block();
    builder->set_insert_point(bb);

    node.function_body->accept(*this);

    if (!is_terminal_stmt)
    {
        if (function_nest.top()->get_return_type() != GET_VOID)
            throw "Non void function without return statement is forbidden";
        else
        {
            is_terminal_stmt = true;
            builder->create_void_ret();
        }
    }
    var_decl_result.pop();
    builder->set_insert_point(old_bb);
    function_nest.pop();
    is_terminal_stmt = false;
}

/**
 * NOTE NOT for member declarations in structures. 
 */
void CminusfBuilder::visit(ASTVarDefinition &node)
{
    node.var_declaration->accept(*this);
    auto var_declaration = pop(var_decl_result); // NOTE consumed

    ValueInfo init_value;
    if (node.init_value.has_value())
    {
        node.init_value.value()->accept(*this);
        init_value = pop(calculation_stack);
    }

    // TODO Support `auto`

    if (var_declaration.id.has_value())
        create_construct(var_declaration.id.value(), var_declaration.type, init_value);
}

void CminusfBuilder::visit(ASTClassTemplateDeclaration &node)
{
    // Certification of template parameter name validation
    for (auto &type_name : node.typenames)
    {
        assert(scalar_type_table.find(type_name) == scalar_type_table.end());
        assert(module->get_struct_type(type_name) == nullptr);
    }

    template_table[node.template_body->struct_id.value()] = &node;
}

/**
 * @brief Push to type_specifier_res
 */
void CminusfBuilder::visit(ASTStructSpecification &node)
{
    auto struct_id = node.struct_id.value_or("anonymous_struct." + to_string(anonymous_struct_name_cnt++));
    if (template_table.find(struct_id) != template_table.end())
        struct_id += "." + to_string(template_cnt++);
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
    if (scalar_type_table.find(node.type_name) != scalar_type_table.end())
    {
        push(type_specifier_res);
        type_specifier_res.top().type = scalar_type_table[node.type_name];
    }
    else if (node.args.has_value())
    {
        assert(template_table.find(node.type_name) != template_table.end());
        auto class_template = template_table[node.type_name];
        auto template_name = class_template->template_body->struct_id.value();
        assert(class_template->typenames.size() == node.args.value().size());
        size_t arg_num = node.args.value().size();
        push(template_parameter_mapping);
        for (size_t i = 0; i < arg_num; i++)
        {
            node.args.value()[i]->accept(*this);
            template_parameter_mapping.top()[class_template->typenames[i]] = pop(type_specifier_res).type;
        }
        auto initiated_template = initiated_templates.find({template_name, template_parameter_mapping.top()});
        if (initiated_template != initiated_templates.end())
        {
            push(type_specifier_res);
            type_specifier_res.top().type = initiated_template->second;
        }
        else
        {
            class_template->template_body->accept(*this);
            initiated_templates[{template_name, template_parameter_mapping.top()}] = dynamic_cast<StructType *>(type_specifier_res.top().type);
        }
        pop(template_parameter_mapping);
    }
    else if (module->get_struct_type(node.type_name))
    {
        push(type_specifier_res);
        type_specifier_res.top().type = module->get_struct_type(node.type_name);
    }
    else if (!template_parameter_mapping.empty() && template_parameter_mapping.top().find(node.type_name) != template_parameter_mapping.top().end())
    {
        push(type_specifier_res);
        type_specifier_res.top().type = template_parameter_mapping.top()[node.type_name];
    }
    else
    {
        throw "Unrecognized type";
    }
}

void CminusfBuilder::visit(ASTCompoundStmt &node)
{
    scope.enter();
    bool this_is_func_body = compound_stmt_is_func_body;
    compound_stmt_is_func_body = false;
    if (this_is_func_body) // This compoundStmt is a function body
    {
        auto param_names = var_decl_result.top().func_param_name.value();
        auto param_name_iter = param_names.begin();
        auto param_type_iter = function_nest.top()->get_function_type()->get_params().begin();
        auto arg_iter = function_nest.top()->get_args().begin();
        for (; arg_iter != function_nest.top()->get_args().end(); arg_iter++, param_name_iter++, param_type_iter++)
            create_construct(param_name_iter->value(), *param_type_iter, {*arg_iter, (*param_type_iter)->is_struct_type()});
    }
    for (auto &&stmt : node.statement_list)
    {
        stmt->accept(*this);
        if (is_terminal_stmt)
            break;
    }
    scope.exit();
}

void CminusfBuilder::visit(ASTExpressionStmt &node)
{
    if (node.expression.has_value())
        node.expression.value()->accept(*this);
}

void CminusfBuilder::visit(ASTSelectionStmt &node)
{
    auto exitBB = new_basic_block();
    auto trueBB = new_basic_block();
    auto falseBB = node.else_statement ? new_basic_block() : exitBB;
    node.expression->accept(*this);
    auto cond_res = calculation_stack.top();
    calculation_stack.pop();
    // compulsiveTypeConvert(cond_res, GET_BOOL);
    builder->create_cond_br(get_r_value(cond_res), trueBB, falseBB);
    builder->set_insert_point(trueBB);
    node.if_statement->accept(*this);
    auto if_stmt_terminate = is_terminal_stmt;
    if (!is_terminal_stmt)
        builder->create_br(exitBB);
    is_terminal_stmt = false;
    if (node.else_statement.has_value())
    {
        builder->set_insert_point(falseBB);
        node.else_statement.value()->accept(*this);
        if (!is_terminal_stmt)
            builder->create_br(exitBB);
        is_terminal_stmt = is_terminal_stmt && if_stmt_terminate;
    }
    if (!is_terminal_stmt)
        builder->set_insert_point(exitBB);
    else
        function_nest.top()->remove(exitBB);
}

void CminusfBuilder::visit(ASTIterationStmt &node)
{
    auto condBB = new_basic_block();
    auto loopBodyBB = new_basic_block();
    auto exitBB = new_basic_block();
    builder->create_br(condBB);
    builder->set_insert_point(condBB);
    node.expression->accept(*this);
    auto cond_res = calculation_stack.top();
    calculation_stack.pop();
    // compulsiveTypeConvert(cond_res, GET_BOOL);
    builder->create_cond_br(get_r_value(cond_res), loopBodyBB, exitBB);
    builder->set_insert_point(loopBodyBB);
    node.statement->accept(*this);
    if (is_terminal_stmt)
        is_terminal_stmt = false;
    else
        builder->create_br(condBB);

    builder->set_insert_point(exitBB);
}

void CminusfBuilder::visit(ASTReturnStmt &node)
{
    if (node.expression.has_value())
    {
        node.expression.value()->accept(*this);
        auto ret_value = pop(calculation_stack);
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
    is_terminal_stmt = true;
}

void CminusfBuilder::visit(ASTVar &node)
{
    auto var = scope.find(node.var_id);
    if (var->get_type()->is_function_type())
    {
        calculation_stack.push({var, false});
    }
    else if (var->get_type()->get_pointer_element_type()->is_array_type()) // is of array type
    {
        auto firstElemAddr = builder->create_gep(var, {CONST_INT(0), CONST_INT(0)});
        calculation_stack.push({firstElemAddr, false});
    }
    else if (var->get_type()->get_pointer_element_type()->is_reference())
    {
        calculation_stack.push({builder->create_load(var), true});
    }
    else
        calculation_stack.push({var, true});
}

void CminusfBuilder::visit(ASTNum &node)
{
    Value *num;
    if (node.num_type == TYPE_INT)
        num = CONST_INT(node.num.i);
    else
        num = CONST_FP(node.num.f);
    calculation_stack.push(ValueInfo{num});
}

void CminusfBuilder::visit(ASTReinterpretCast &node)
{
    node.obj_type->accept(*this);
    auto obj_type = pop(var_decl_result);
    node.src_expression->accept(*this);
    auto src_expressin = get_r_value(pop(calculation_stack));
    calculation_stack.push({builder->create_bitcast(src_expressin, obj_type.type)});
}

void CminusfBuilder::visit(ASTCall &node)
{
    node.callee->accept(*this);
    auto callee = pop(calculation_stack);
    std::vector<ValueInfo> args;
    for (auto arg : node.args)
    {
        arg->accept(*this);
        args.push_back(pop(calculation_stack));
    }

    calculation_stack.push(create_call(callee, std::move(args)));
}

void CminusfBuilder::visit(ASTSubscript &node)
{
    node.array->accept(*this);
    auto array = pop(calculation_stack);
    node.subscript->accept(*this);
    auto subscript = pop(calculation_stack);
    calculation_stack.push(create_subscript(array, subscript));
}

void CminusfBuilder::visit(ASTMemberAccess &node)
{
    node.object->accept(*this);
    auto object = pop(calculation_stack);
    calculation_stack.push(create_member_access(object, node.member_id));
}

void CminusfBuilder::visit(ASTUnaryAddExpression &node)
{
    // TODO Unary add operation
    throw "Unary additive operators are not implemented yet";
}

void CminusfBuilder::visit(ASTDereference &node)
{
    node.expression->accept(*this);
    auto operand = pop(calculation_stack);
    calculation_stack.push(create_dereference(operand));
}

void CminusfBuilder::visit(ASTAddressof &node)
{
    node.expression->accept(*this);
    auto operand = pop(calculation_stack);
    calculation_stack.push(create_addressof(operand));
}

void CminusfBuilder::visit(ASTMultiplicativeExpression &node)
{
    node.l_expression->accept(*this);
    auto loperand = pop(calculation_stack);
    node.r_expression->accept(*this);
    auto roperand = pop(calculation_stack);
    calculation_stack.push(create_mul(loperand, roperand, node.op));
}

void CminusfBuilder::visit(ASTAdditiveExpression &node)
{
    node.l_expression->accept(*this);
    auto loperand = pop(calculation_stack);
    node.r_expression->accept(*this);
    auto roperand = pop(calculation_stack);
    calculation_stack.push(create_add(loperand, roperand, node.op));
}

void CminusfBuilder::visit(ASTRelationalExpression &node)
{
    node.l_expression->accept(*this);
    auto loperand = pop(calculation_stack);
    node.r_expression->accept(*this);
    auto roperand = pop(calculation_stack);
    calculation_stack.push(create_rel(loperand, roperand, node.op));
}

void CminusfBuilder::visit(ASTAssignExpression &node)
{
    node.l_expression->accept(*this);
    auto loperand = pop(calculation_stack);
    node.r_expression->accept(*this);
    auto roperand = pop(calculation_stack);
    assert(loperand.is_lvalue);
    calculation_stack.push(create_assign(loperand.value, roperand));
}
