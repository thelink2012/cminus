#include <cminus/scanner.hpp>
#include <memory>

namespace cminus
{
class AST;
class ASTExpr;
class ASTBinaryExpr;
class ASTAssignExpr;
class ASTNumber;
class ASTVar;
class ASTCall;

/// Base of any AST node.
class AST : public std::enable_shared_from_this<AST>
{
public:
    virtual ~AST() {}

    /// Dumps the node in labeled bracket notation.
    virtual void dump(std::string&, size_t depth) = 0;

    /// \returns whether this node is of type T.
    template<typename T>
    bool is() const
    {
        // TODO use static information instead of RTTI.
        if(dynamic_cast<std::add_pointer_t<const T>>(this))
            return true;
        return false;
    }
};

/// Base of any expression node.
class ASTExpr : public AST
{
public:
    virtual void dump(std::string&, size_t depth) = 0;
};

/// Node of a binary expression in the AST.
class ASTBinaryExpr : public ASTExpr
{
public:
    enum class Operation
    {
        Plus,
        Minus,
        Multiply,
        Divide,
        Less,
        LessEqual,
        Greater,
        GreaterEqual,
        Equal,
        NotEqual,
        Assign,
    };

public:
    explicit ASTBinaryExpr(std::shared_ptr<ASTExpr> left,
                           std::shared_ptr<ASTExpr> right,
                           Operation op) :
        left(std::move(left)),
        right(std::move(right)), op(op)
    {
        assert(this->left != nullptr && this->right != nullptr);
    }

    virtual void dump(std::string&, size_t depth);

    /// Converts an word category into a operation enumeration.
    static Operation type_from_category(Category category);

private:
    std::shared_ptr<ASTExpr> left;
    std::shared_ptr<ASTExpr> right;
    Operation op;
};

/// Node of an assignment expression.
class ASTAssignExpr : public ASTBinaryExpr
{
public:
    explicit ASTAssignExpr(std::shared_ptr<ASTExpr> left,
                           std::shared_ptr<ASTExpr> right) :
        ASTBinaryExpr(std::move(left), std::move(right), Operation::Assign)
    {
        assert(left->is<ASTVar>());
    }
};

/// Node of a number.
class ASTNumber : public ASTExpr
{
public:
    explicit ASTNumber(int32_t number) :
        value(number)
    {
    }

    virtual void dump(std::string&, size_t depth);

private:
    int32_t value;
};

/// Node of a variable in the AST.
class ASTVar : public ASTExpr
{
public:
    virtual void dump(std::string&, size_t depth);
};

/// Node of a function call in the AST.
class ASTCall : public ASTExpr
{
public:
    virtual void dump(std::string&, size_t depth);
};
}
