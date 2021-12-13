#include "cminusf_builder.hpp"

using namespace std::placeholders;
using namespace std;

void CminusfBuilder::visit(ASTProgram &node)
{
    std::for_each(node.definitions.begin(), node.definitions.end(), [&](auto &ptr)
                  { ptr->accept(*this); });
}

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
    assert(function_prototype.id.has_value());
    assert(all_of(latest_func_def_param_names.begin(), latest_func_def_param_names.end(), [&](auto id)
                  { return id.has_value(); }));
    if (!struct_nest.empty())
    {
        auto orig_func_ty = dynamic_cast<FunctionType *>(function_prototype.type);
        vector<Type *> params{struct_nest.back()};
        for (auto &&param_ty : orig_func_ty->get_params())
            params.push_back(param_ty);
        function_prototype.type = FunctionType::get(orig_func_ty->get_return_type(), params, MOD);
        function_prototype.id = get_struct_id_prefix() + function_prototype.id.value();
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

void CminusfBuilder::visit(ASTVarDefinition &node)
{
    cerr << "Enter ASTVarDefinition\n";
    
}

void CminusfBuilder::visit(ASTStructSpecification &node)
{
    cerr << "Enter ASTStructSpecification\n";
}

void CminusfBuilder::visit(ASTNamedType &node)
{
    cerr << "Enter ASTNamedType\n";
}

void CminusfBuilder::visit(ASTCompoundStmt &node)
{
    cerr << "Enter ASTCompoundStmt\n";
    scope.enter();
    bool this_is_func_body = compound_stmt_is_func_body;
    if (compound_stmt_is_func_body) // This compoundStmt is a function body
    {
        auto param_name_iter = latest_func_def_param_names.begin();
        for (auto arg : function_nest.top()->get_args())
        {
            auto alloca = builder->create_alloca(get_valid_param_ty(builder, node));
            scope.push(param_name_iter++->value(), alloca);
            builder->create_store(arg, alloca);
        }
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
}

void CminusfBuilder::visit(ASTVar &node)
{
    cerr << "Enter ASTVar\n";
}

void CminusfBuilder::visit(ASTNum &node)
{
    cerr << "Enter ASTNum\n";
}

void CminusfBuilder::visit(ASTCall &node)
{
    cerr << "Enter ASTCall\n";
}

void CminusfBuilder::visit(ASTSubscript &node)
{
    cerr << "Enter ASTSubscript\n";
}

void CminusfBuilder::visit(ASTMemberAccess &node)
{
    cerr << "Enter ASTMemberAccess\n";
}

void CminusfBuilder::visit(ASTUnaryAddExpression &node)
{
    cerr << "Enter ASTUnaryAddExpression\n";
}

void CminusfBuilder::visit(ASTDereference &node)
{
    cerr << "Enter ASTDereference\n";
}

void CminusfBuilder::visit(ASTAddressof &node)
{
    cerr << "Enter ASTAddressof\n";
}

void CminusfBuilder::visit(ASTMultiplicativeExpression &node)
{
    cerr << "Enter ASTMultiplicativeExpression\n";
}

void CminusfBuilder::visit(ASTAdditiveExpression &node)
{
    cerr << "Enter ASTAdditiveExpression\n";
}

void CminusfBuilder::visit(ASTRelationalExpression &node)
{
    cerr << "Enter ASTRelationalExpression\n";
}

void CminusfBuilder::visit(ASTAssignExpression &node)
{
    cerr << "Enter ASTAssignExpression\n";
}
