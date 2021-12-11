#ifndef _SYNTAX_TREE_HPP_
#define _SYNTAX_TREE_HPP_
extern "C"
{
#include "syntax_tree.h"
    extern syntax_tree *parse(const char *input);
}
#include <vector>
#include <memory>
#include <string>
#include <optional>

enum CminusType
{
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_VOID
};

enum RelOp
{
    // <=
    OP_LE,
    // <
    OP_LT,
    // >
    OP_GT,
    // >=
    OP_GE,
    // ==
    OP_EQ,
    // !=
    OP_NEQ
};

enum AddOp
{
    // +
    OP_PLUS,
    // -
    OP_MINUS
};

enum MulOp
{
    // *
    OP_MUL,
    // /
    OP_DIV
};

class AST;

struct ASTNode;

// Program structure
struct ASTProgram;

// Definitions & declarations
struct ASTVarDeclaration;

struct ASTDeclarationExpression : ASTNode
{
};
struct ASTDeclarationDereference;
struct ASTDeclarationCall;
struct ASTDeclarationSubscript;

struct ASTDefinition : ASTNode
{
};
struct ASTFunDefinition;
struct ASTVarDefinition;

struct ASTTypeSpecifier : ASTNode
{
};
struct ASTStructSpecification;
struct ASTNamedType;

// Statements
struct ASTStatement : ASTNode
{
};
struct ASTCompoundStmt;
struct ASTParam;
struct ASTExpressionStmt;
struct ASTSelectionStmt;
struct ASTIterationStmt;
struct ASTReturnStmt;
// VarDefinition is also a statement

// Expression
struct ASTExpression : ASTNode
{
};
struct ASTVar;
struct ASTNum;
struct ASTCall;
struct ASTSubscript;
struct ASTMemberAccess;
struct ASTUnaryAddExpression;
struct ASTDereference;
struct ASTAddressof;
struct ASTMultiplicativeExpression;
struct ASTAdditiveExpression;
struct ASTRelationalExpression;
struct ASTAssignExpression;

class ASTVisitor;

class AST
{
public:
    AST() = delete;
    AST(syntax_tree *);
    AST(AST &&tree)
    {
        root = tree.root;
        tree.root = nullptr;
    };
    ASTProgram *get_root() { return root.get(); }
    void run_visitor(ASTVisitor &visitor);

private:
    ASTNode *transform_node_iter(syntax_tree_node *);
    std::shared_ptr<ASTProgram> root = nullptr;
};

struct ASTNode
{
    virtual void accept(ASTVisitor &) = 0;
};

// Program structure
struct ASTProgram : ASTNode
{
    virtual void accept(ASTVisitor &) override final;
    std::vector<std::shared_ptr<ASTDefinition>>
        definitions;
};

// Definitions & declarations

struct ASTVarDeclaration : ASTNode
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTTypeSpecifier> type_specifier;
    std::shared_ptr<ASTDeclarationExpression> decl_expression;
};

struct ASTDeclarationDereference : ASTDeclarationExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTDeclarationExpression> expression;
};

struct ASTDeclarationCall : ASTDeclarationExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTDeclarationExpression> caller;
    std ::vector<std::shared_ptr<ASTVarDeclaration>> params;
};
struct ASTDeclarationSubscript : ASTDeclarationExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTDeclarationExpression> array;
    std::optional<int> subscript;
};

struct ASTFunDefinition : ASTDefinition
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTVarDeclaration> var_declaration;
    std::shared_ptr<ASTCompoundStmt> function_body;
};

struct ASTVarDefinition : ASTDefinition, ASTStatement
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTVarDeclaration> var_declaration;
    std::optional<ASTExpression> init_value;
};

struct ASTStructSpecification : ASTTypeSpecifier
{
    virtual void accept(ASTVisitor &) override final;
    std::string struct_id;
    std::vector<ASTDefinition> definitions;
};

struct ASTNamedType : ASTTypeSpecifier
{
    virtual void accept(ASTVisitor &) override final;
    std::string type_name;
};

// Statements
struct ASTCompoundStmt : ASTStatement
{
    virtual void accept(ASTVisitor &) override final;
    std::vector<std::shared_ptr<ASTVarDefinition>> declarations;
    std::vector<std::shared_ptr<ASTStatement>> statement_list;
};

struct ASTExpressionStmt : ASTStatement
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTExpression> expression;
};

struct ASTSelectionStmt : ASTStatement
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTExpression> expression;
    std::shared_ptr<ASTStatement> if_statement;

    std::optional<std::shared_ptr<ASTStatement>> else_statement;
};

struct ASTIterationStmt : ASTStatement
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTExpression> expression;
    std::shared_ptr<ASTStatement> statement;
};

struct ASTReturnStmt : ASTStatement
{
    virtual void accept(ASTVisitor &) override final;
    std::optional<std::shared_ptr<ASTExpression>> expression;
};

// Expressions
struct ASTVar : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::string var_id;
};
struct ASTNum : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    enum CminusType num_type;
    union
    {
        float f;
        int i;
    } num;
};
struct ASTCall : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTExpression> caller;
    std ::vector<std::shared_ptr<ASTVarDeclaration>> params
};
struct ASTSubscript : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTExpression> array;
    std::shared_ptr<ASTExpression> subscript;
};
struct ASTMemberAccess : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTExpression> object;
    std::string member_id;
};
struct ASTUnaryAddExpression : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    enum AddOp op;
    std::shared_ptr<ASTExpression> expression;
};
struct ASTDereference : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTExpression> expression;
};
struct ASTAddressof : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTExpression> expression;
};
struct ASTMultiplicativeExpression : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    enum MulOp op;
    std::shared_ptr<ASTExpression> l_expression;
    std::shared_ptr<ASTExpression> r_expression;
};
struct ASTAdditiveExpression : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    enum AddOp op;
    std::shared_ptr<ASTExpression> l_expression;
    std::shared_ptr<ASTExpression> r_expression;
};
struct ASTRelationalExpression : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    enum RelOp op;
    std::shared_ptr<ASTExpression> l_expression;
    std::shared_ptr<ASTExpression> r_expression;
};
struct ASTAssignExpression : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTExpression> l_expression;
    std::shared_ptr<ASTExpression> r_expression;
};


class ASTVisitor
{
public:
    virtual void visit(ASTProgram &) = 0;
    virtual void visit(ASTDeclarationExpression &) = 0;
    virtual void visit(ASTDeclarationDereference &) = 0;
    virtual void visit(ASTDeclarationCall &) = 0;
    virtual void visit(ASTDeclarationSubscript &) = 0;
    virtual void visit(ASTFunDefinition &) = 0;
    virtual void visit(ASTVarDefinition &) = 0;
    virtual void visit(ASTStructSpecification &) = 0;
    virtual void visit(ASTNamedType &) = 0;
    virtual void visit(ASTCompoundStmt &) = 0;
    virtual void visit(ASTParam &) = 0;
    virtual void visit(ASTExpressionStmt &) = 0;
    virtual void visit(ASTSelectionStmt &) = 0;
    virtual void visit(ASTIterationStmt &) = 0;
    virtual void visit(ASTReturnStmt &) = 0;
    virtual void visit(ASTVar &) = 0;
    virtual void visit(ASTNum &) = 0;
    virtual void visit(ASTCall &) = 0;
    virtual void visit(ASTSubscript &) = 0;
    virtual void visit(ASTMemberAccess &) = 0;
    virtual void visit(ASTUnaryAddExpression &) = 0;
    virtual void visit(ASTDereference &) = 0;
    virtual void visit(ASTAddressof &) = 0;
    virtual void visit(ASTMultiplicativeExpression &) = 0;
    virtual void visit(ASTAdditiveExpression &) = 0;
    virtual void visit(ASTRelationalExpression &) = 0;
    virtual void visit(ASTAssignExpression &) = 0;
};
#endif
