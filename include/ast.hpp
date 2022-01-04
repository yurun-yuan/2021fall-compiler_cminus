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
#include <cstring>
#include <stack>
#include <iostream>
#include <iterator>
#include <unordered_map>

#define STR_EQ(a, b) (strcmp((a), (b)) == 0)
#define CHILD(num) ((n)->children[(num)])
#define CASE0(s) if (STR_EQ((n->name), s))
#define CASE(s) else if (STR_EQ((n->name), s))
#define CASE_CHILD0(num, s) if (n->children_num > (num) && STR_EQ((n)->children[(num)]->name, s))
#define CASE_CHILD(num, s) else if (n->children_num > (num) && STR_EQ((n)->children[(num)]->name, s))
#define DEFAULT else
#define SHARED(TYPE, ptr) std::shared_ptr<TYPE>(static_cast<TYPE *>(ptr))
#define Some make_optional
#define END return node;
#define NEW(TYPE) auto node = new TYPE()
#define EXCEPT_DEFAULT   \
    else                 \
    {                    \
        _AST_NODE_ERROR_ \
    }

using namespace std;

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
class ASTVisitor;

struct ASTNode
{
    virtual void accept(ASTVisitor &) = 0;
};

// Program structure
struct ASTProgram;

// Definitions & declarations

struct ASTDeclarationExpression : ASTNode
{
    virtual void accept(ASTVisitor &) = 0;
    virtual ~ASTDeclarationExpression() = default;
};
struct ASTVarDeclaration;
struct ASTDeclarationIdentifier;
struct ASTDeclarationDereference;
struct ASTDeclarationCall;
struct ASTDeclarationSubscript;

struct ASTDefinition : ASTNode
{
    virtual void accept(ASTVisitor &) = 0;
    virtual ~ASTDefinition() = default;
};
struct ASTFunDefinition;
struct ASTVarDefinition;
struct ASTClassTemplateDeclaration;

struct ASTTypeSpecifier : ASTNode
{
    virtual void accept(ASTVisitor &) = 0;
    virtual ~ASTTypeSpecifier() = default;
};
struct ASTStructSpecification;
struct ASTNamedType;

// Statements
struct ASTStatement : ASTNode
{
    virtual void accept(ASTVisitor &) = 0;
    virtual ~ASTStatement() = default;
};
struct ASTCompoundStmt;
struct ASTExpressionStmt;
struct ASTSelectionStmt;
struct ASTIterationStmt;
struct ASTReturnStmt;
// VarDefinition is also a statement

// Expression
struct ASTExpression : ASTNode
{
    virtual void accept(ASTVisitor &) = 0;
    virtual ~ASTExpression() = default;
};
struct ASTVar;
struct ASTNum;
struct ASTReinterpretCast;
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
    ASTNode *transform(syntax_tree_node *);
    std::shared_ptr<ASTProgram> root = nullptr;
    template <typename Array, typename Func>
    void flatten(
        syntax_tree_node *root, Array &array,
        Func func,
        int l_child_idx = 0,
        int r_child_idx = 1,
        int leaf_child_idx = 0)
    {
        if (root->children_num == 0)
            return;

        std::stack<syntax_tree_node *> s;
        while (root->children_num > r_child_idx)
        {
            s.push(root->children[r_child_idx]);
            root = root->children[l_child_idx];
        }

        s.push(root->children[leaf_child_idx]);

        while (!s.empty())
        {
            array.push_back(func(s.top()));
            s.pop();
        }
    }
    template <typename T>
    void bi_operation_helper(T &node, syntax_tree_node *n)
    {
        node->l_expression = SHARED(ASTExpression, transform(CHILD(0)));
        node->r_expression = SHARED(ASTExpression, transform(CHILD(2)));
    }

    auto add_op(const char *op_name)
    {
        if (STR_EQ(op_name, "+"))
            return OP_PLUS;
        else if (STR_EQ(op_name, "-"))
            return OP_MINUS;
        else
            throw "Invalid op";
    }
    auto mul_op(const char *op_name)
    {
        if (STR_EQ(op_name, "*"))
            return OP_MUL;
        else if (STR_EQ(op_name, "/"))
            return OP_DIV;
        else
            throw "Invalid op";
    }
    auto rel_op(const char *op_name)
    {
        if (STR_EQ(op_name, "<="))
            return OP_LE;
        else if (STR_EQ(op_name, "<"))
            return OP_LT;
        else if (STR_EQ(op_name, ">"))
            return OP_GT;
        else if (STR_EQ(op_name, ">="))
            return OP_GE;
        else if (STR_EQ(op_name, "=="))
            return OP_EQ;
        else if (STR_EQ(op_name, "!="))
            return OP_NEQ;
        else
            throw "Invalid op";
    }
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

struct ASTDeclarationIdentifier : ASTDeclarationExpression
{
    virtual void accept(ASTVisitor &) override final;
    bool is_ref = false;
    std::optional<std::string> id;
    std::optional<char> operator_load;
};

struct ASTDeclarationDereference : ASTDeclarationExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTDeclarationExpression> expression;
};

struct ASTDeclarationCall : ASTDeclarationExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTDeclarationExpression> callee;
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
    std::optional<std::shared_ptr<ASTExpression>> init_value;
};

struct ASTClassTemplateDeclaration : ASTDefinition
{
    virtual void accept(ASTVisitor &) override final;
    std::vector<std::string> typenames;
    std::shared_ptr<ASTStructSpecification> template_body;
};

struct ASTStructSpecification : ASTTypeSpecifier
{
    virtual void accept(ASTVisitor &) override final;
    std::optional<std::string> struct_id;
    std::vector<shared_ptr<ASTDefinition>> definitions;
};

struct ASTNamedType : ASTTypeSpecifier
{
    virtual void accept(ASTVisitor &) override final;
    std::string type_name;
    optional<vector<shared_ptr<ASTTypeSpecifier>>> args;
};

// Statements
struct ASTCompoundStmt : ASTStatement
{
    virtual void accept(ASTVisitor &) override final;
    std::vector<std::shared_ptr<ASTStatement>> statement_list;
};

struct ASTExpressionStmt : ASTStatement
{
    virtual void accept(ASTVisitor &) override final;
    std::optional<std::shared_ptr<ASTExpression>> expression;
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
struct ASTReinterpretCast : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTVarDeclaration> obj_type;
    std::shared_ptr<ASTExpression> src_expression;
};
struct ASTCall : ASTExpression
{
    virtual void accept(ASTVisitor &) override final;
    std::shared_ptr<ASTExpression> callee;
    std::vector<std::shared_ptr<ASTExpression>> args;
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
    virtual void visit(ASTVarDeclaration &) = 0;
    virtual void visit(ASTDeclarationIdentifier &) = 0;
    virtual void visit(ASTDeclarationDereference &) = 0;
    virtual void visit(ASTDeclarationCall &) = 0;
    virtual void visit(ASTDeclarationSubscript &) = 0;
    virtual void visit(ASTFunDefinition &) = 0;
    virtual void visit(ASTVarDefinition &) = 0;
    virtual void visit(ASTClassTemplateDeclaration &) = 0;
    virtual void visit(ASTStructSpecification &) = 0;
    virtual void visit(ASTNamedType &) = 0;
    virtual void visit(ASTCompoundStmt &) = 0;
    virtual void visit(ASTExpressionStmt &) = 0;
    virtual void visit(ASTSelectionStmt &) = 0;
    virtual void visit(ASTIterationStmt &) = 0;
    virtual void visit(ASTReturnStmt &) = 0;
    virtual void visit(ASTVar &) = 0;
    virtual void visit(ASTNum &) = 0;
    virtual void visit(ASTReinterpretCast &) = 0;
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
