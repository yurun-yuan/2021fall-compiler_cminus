#include "ast.hpp"

#define _AST_NODE_ERROR_                         \
    std::cerr << "Abort due to node cast error." \
              << std::endl;                      \
    std::abort();

using namespace std;

void AST::run_visitor(ASTVisitor &visitor)
{
    root->accept(visitor);
}

AST::AST(syntax_tree *s)
{
    if (s == nullptr)
    {
        std::cerr << "empty input tree!" << std::endl;
        std::abort();
    }
    auto node = transform(s->root);
    del_syntax_tree(s);
    root = std::shared_ptr<ASTProgram>(
        static_cast<ASTProgram *>(node));
}

ASTNode *AST::transform(syntax_tree_node *n)
{

    /**
     * @brief Program Structure
     * 
     */
    CASE0("program")
    {
        NEW(ASTProgram);

        // flatten definition list
        flatten(n->children[0], node->definitions, [&](syntax_tree_node *node)
                { return SHARED(ASTDefinition, transform(node)); });
        END;
    }
    CASE("definition")
    {
        return transform(n->children[0]);
    }
    /**
     * @brief Definitions and Declarations
     * 
     */
    CASE("var-definition")
    {
        NEW(ASTVarDefinition);

        node->var_declaration = SHARED(ASTVarDeclaration, transform(n->children[0]));

        CASE_CHILD0(1, "=")
        {
            node->init_value = SHARED(ASTExpression, transform(CHILD(2)));
        }

        if (STR_EQ(n->parent->name, "statement"))
            return static_cast<ASTStatement *>(node);
        else
            return static_cast<ASTDefinition *>(node);
    }
    CASE("fun-definition")
    {
        NEW(ASTFunDefinition);

        node->var_declaration = SHARED(ASTVarDeclaration, transform(CHILD(0)));
        node->function_body = SHARED(ASTCompoundStmt, transform(CHILD(1)));
        END;
    }
    CASE("class-template-declaration")
    {
        NEW(ASTClassTemplateDeclaration);

        vector<string> param_list;
        flatten(
            CHILD(2), param_list, [&](syntax_tree_node *node)
            { return node->name; },
            0, 3, 1);
        node->typenames = param_list;
        node->template_body = SHARED(ASTStructSpecification, transform(CHILD(4)));
        END;
    }
    CASE("struct-definition")
    {
        NEW(ASTStructSpecification);
        int definitions_index;
        CASE_CHILD0(1, "{")
        {
            definitions_index = 2;
        }
        DEFAULT
        {
            node->struct_id = CHILD(1)->name;
            definitions_index = 3;
        }

        if (CHILD(definitions_index)->children_num != 0)
        {
            flatten(CHILD(definitions_index)->children[0], node->definitions, [&](syntax_tree_node *node)
                    { return SHARED(ASTDefinition, transform(node)); });
        }
        END;
    }
    CASE("scalar-type-specifier")
    {
        NEW(ASTNamedType);
        node->type_name = CHILD(0)->name;
        END;
    }
    CASE("type-specifier")
    {
        CASE_CHILD0(0, "struct-definition")
        {
            return transform(CHILD(0));
        }
        CASE_CHILD(0, "scalar-type-specifier")
        {
            return transform(CHILD(0));
        }
        CASE_CHILD(3, "template-arg-list")
        {
            NEW(ASTNamedType);
            node->type_name = CHILD(1)->name;
            vector<shared_ptr<ASTTypeSpecifier>> args;
            flatten(
                CHILD(3), args, [&](syntax_tree_node *node)
                { return SHARED(ASTTypeSpecifier, transform(node)); },
                0, 2);
            node->args = args;
            END;
        }
        CASE_CHILD(0, "struct")
        {
            NEW(ASTNamedType);
            node->type_name = CHILD(1)->name;
            END;
        }
        CASE_CHILD(0, "typename")
        {
            NEW(ASTNamedType);
            node->type_name = CHILD(1)->name;
            END;
        }
        EXCEPT_DEFAULT;
    }
    CASE("var-declaration")
    {
        NEW(ASTVarDeclaration);
        node->type_specifier = SHARED(ASTTypeSpecifier, transform(CHILD(0)));
        node->decl_expression = SHARED(ASTDeclarationExpression, transform(CHILD(1)));
        END;
    }
    CASE("var-decl-expression")
    {
        CASE_CHILD0(0, "*")
        {
            NEW(ASTDeclarationDereference);
            node->expression = SHARED(ASTDeclarationExpression, transform(CHILD(1)));
            END;
        }
        DEFAULT
        {
            return transform(CHILD(0));
        }
    }
    CASE("var-decl-element")
    {
        CASE_CHILD0(2, "params")
        {
            NEW(ASTDeclarationCall);
            node->callee = SHARED(ASTDeclarationExpression, transform(CHILD(0)));
            auto params = CHILD(2);
            if (params->children_num != 0 && !STR_EQ(params->children[0]->name, "void"))
            {
                flatten(
                    params->children[0], node->params, [&](syntax_tree_node *node)
                    { return SHARED(ASTVarDeclaration, transform(node)); },
                    0,
                    2);
            }
            END;
        }
        CASE_CHILD(2, "integer")
        {
            NEW(ASTDeclarationSubscript);
            node->array = SHARED(ASTDeclarationExpression, transform(CHILD(0)));
            node->subscript = std::stoi(CHILD(2)->children[0]->name);
            END;
        }
        CASE_CHILD(0, "var-decl-atom")
        {
            return transform(CHILD(0));
        }
        else
        {
            NEW(ASTDeclarationSubscript);
            node->array = SHARED(ASTDeclarationExpression, transform(CHILD(0)));
            END;
        }
    }
    CASE("var-decl-atom")
    {
        CASE_CHILD0(0, "var-reference")
        {
            NEW(ASTDeclarationIdentifier);
            auto var_ref = CHILD(0);
            if (var_ref->children_num == 2 && var_ref->children[0]->name == string("operator"))
            {
                node->operator_load = var_ref->children[1]->children[0]->name[0];
                node->id = string(var_ref->children[0]->name) + node->operator_load.value();
            }
            else if (var_ref->children_num == 0) // Not reference, no var name
            {
                ;
            }
            else if (var_ref->children_num == 2) // Is reference, has var name
            {
                node->is_ref = true;
                node->id = var_ref->children[1]->name;
            }
            else if (STR_EQ(var_ref->children[0]->name, "&")) // Is reference, no var name
            {
                node->is_ref = true;
            }
            else // Not reference, has var name
            {
                node->id = var_ref->children[0]->name;
            }
            END;
        }
        DEFAULT
        {
            return transform(CHILD(1));
        }
    }

    // Statements
    CASE("compound-stmt")
    {
        NEW(ASTCompoundStmt);
        CASE_CHILD0(1, "statements")
        {
            auto statements = CHILD(1);
            if (statements->children_num != 0)
            {
                flatten(statements->children[0], node->statement_list, [&](syntax_tree_node *node)
                        { return SHARED(ASTStatement, transform(node)); });
            }
        }
        EXCEPT_DEFAULT;
        END;
    }
    CASE("statement")
    {
        return transform(n->children[0]);
    }
    CASE("expression-stmt")
    {
        NEW(ASTExpressionStmt);
        if (n->children_num == 2)
            node->expression = SHARED(ASTExpression, transform(CHILD(0)));
        END;
    }
    CASE("selection-stmt")
    {
        NEW(ASTSelectionStmt);

        node->expression = SHARED(ASTExpression, transform(CHILD(2)));
        node->if_statement = SHARED(ASTStatement, transform(CHILD(4)));

        if (n->children_num == 7)
        {
            node->else_statement = SHARED(ASTStatement, transform(CHILD(6)));
        }
        END;
    }
    CASE("iteration-stmt")
    {
        NEW(ASTIterationStmt);
        node->expression = SHARED(ASTExpression, transform(CHILD(2)));
        node->statement = SHARED(ASTStatement, transform(CHILD(4)));
        END;
    }
    CASE("return-stmt")
    {
        NEW(ASTReturnStmt);
        CASE_CHILD0(1, "expression")
        {
            node->expression = SHARED(ASTExpression, transform(CHILD(1)));
        }
        END;
    }

    // Expression
    CASE("expression")
    {
        CASE_CHILD0(1, "=")
        {
            NEW(ASTAssignExpression);
            bi_operation_helper(node, n);
            END;
        }
        DEFAULT
        {
            return transform(CHILD(0));
        }
    }
    CASE("relational-expression")
    {
        CASE_CHILD0(1, "relop")
        {
            NEW(ASTRelationalExpression);
            bi_operation_helper(node, n);
            auto op_name = CHILD(1)->children[0]->name;
            node->op = rel_op(op_name);
            END;
        }
        DEFAULT
        {
            return transform(CHILD(0));
        }
    }
    CASE("additive-expression")
    {
        CASE_CHILD0(1, "addop")
        {
            NEW(ASTAdditiveExpression);
            bi_operation_helper(node, n);
            auto op_name = CHILD(1)->children[0]->name;
            node->op = add_op(op_name);
            END;
        }
        DEFAULT
        {
            return transform(CHILD(0));
        }
    }
    CASE("multiplicative-expression")
    {
        CASE_CHILD0(1, "mulop")
        {
            NEW(ASTMultiplicativeExpression);
            bi_operation_helper(node, n);
            auto op_name = CHILD(1)->children[0]->name;
            node->op = mul_op(op_name);
            END;
        }
        DEFAULT
        {
            return transform(CHILD(0));
        }
    }
    CASE("factor")
    {
        CASE_CHILD0(0, "addop")
        {
            NEW(ASTUnaryAddExpression);
            node->op = add_op(CHILD(0)->children[0]->name);
            node->expression = SHARED(ASTExpression, transform(CHILD(1)));
            END;
        }
        CASE_CHILD(0, "*")
        {
            NEW(ASTDereference);
            node->expression = SHARED(ASTExpression, transform(CHILD(1)));
            END;
        }
        CASE_CHILD(0, "&")
        {
            NEW(ASTAddressof);
            node->expression = SHARED(ASTExpression, transform(CHILD(1)));
            END;
        }
        CASE_CHILD(0, "element")
        {
            return transform(CHILD(0));
        }
        EXCEPT_DEFAULT;
    }
    CASE("element")
    {
        CASE_CHILD0(2, "args")
        {
            NEW(ASTCall);
            node->callee = SHARED(ASTExpression, transform(CHILD(0)));
            auto args = CHILD(2);
            if (args->children_num != 0)
            {
                flatten(
                    args->children[0], node->args,
                    [&](syntax_tree_node *node)
                    { return SHARED(ASTExpression, transform(node)); },
                    0, 2);
            }
            END;
        }
        CASE_CHILD(2, "expression")
        {
            NEW(ASTSubscript);
            node->array = SHARED(ASTExpression, transform(CHILD(0)));
            node->subscript = SHARED(ASTExpression, transform(CHILD(2)));
            END;
        }
        CASE_CHILD(1, ".")
        {
            NEW(ASTMemberAccess);
            node->object = SHARED(ASTExpression, transform(CHILD(0)));
            node->member_id = CHILD(2)->name;
            END;
        }
        DEFAULT
        {
            return transform(CHILD(0));
        }
    }
    CASE("atom")
    {
        CASE_CHILD0(0, "integer")
        {
            NEW(ASTNum);
            node->num_type = TYPE_INT;
            node->num.i = std::stoi(CHILD(0)->children[0]->name);
            END;
        }
        CASE_CHILD(0, "float")
        {
            NEW(ASTNum);
            node->num_type = TYPE_FLOAT;
            node->num.f = std::stof(CHILD(0)->children[0]->name);
            END;
        }
        CASE_CHILD(0, "string")
        {
            // TODO
            throw "Literal string not implemented";
        }
        CASE_CHILD(0, "reinterpret-cast")
        {
            NEW(ASTReinterpretCast);
            auto reinterpret_cast_ = CHILD(0);
            node->obj_type = SHARED(ASTVarDeclaration, transform(reinterpret_cast_->children[2]));
            node->src_expression = SHARED(ASTExpression, transform(reinterpret_cast_->children[5]));
            END;
        }
        else if (n->children_num == 3)
        {
            return transform(CHILD(1));
        }
        DEFAULT
        {
            NEW(ASTVar);
            node->var_id = CHILD(0)->name;
            END;
        }
    }
    DEFAULT
    {
        std::cerr << "[ast]: transform failure!" << std::endl;
        std::abort();
    }
}

void ASTProgram::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTVarDeclaration::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTDeclarationIdentifier::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTDeclarationDereference::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTDeclarationCall::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTDeclarationSubscript::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTFunDefinition::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTVarDefinition::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTClassTemplateDeclaration::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTStructSpecification::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTNamedType::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTCompoundStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTExpressionStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTSelectionStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTIterationStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTReturnStmt::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTVar::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTNum::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTReinterpretCast::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTCall::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTSubscript::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTMemberAccess::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTUnaryAddExpression::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTDereference::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTAddressof::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTMultiplicativeExpression::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTAdditiveExpression::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTRelationalExpression::accept(ASTVisitor &visitor) { visitor.visit(*this); }
void ASTAssignExpression::accept(ASTVisitor &visitor) { visitor.visit(*this); }
