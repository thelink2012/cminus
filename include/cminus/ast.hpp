#include <cminus/scanner.hpp>
#include <memory>

namespace cminus
{
class AST;
class ASTExpr;
class ASTDecl;
class ASTStmt;
class ASTProgram;
class ASTFunDecl;
class ASTBinaryExpr;
class ASTAssignExpr;
class ASTNumber;
class ASTVarRef;
class ASTFunCall;
class ASTCompoundStmt;

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
        // TODO think about hierarchy checking when static!?
        if(dynamic_cast<std::add_pointer_t<const T>>(this))
            return true;
        return false;
    }
};

/// Base of any declaration node.
class ASTDecl : public AST
{
};

// Base of any statement node.
class ASTStmt : public AST
{
};

/// Base of any expression node.
class ASTExpr : public ASTStmt
{
};


/// Node that represents an entire program.
class ASTProgram : public AST
{
public:
    explicit ASTProgram()
    {
    }

    virtual void dump(std::string&, size_t depth);

    /// Adds a new declaration into the program.
    void add_decl(std::shared_ptr<ASTDecl> decl)
    {
        assert(decl != nullptr);
        decls.push_back(std::move(decl));
    }

private:
    std::vector<std::shared_ptr<ASTDecl>> decls;
};

// Node that represents a variable declaration.
class ASTVarDecl : public ASTDecl
{
public:
    explicit ASTVarDecl(SourceRange name,
                        std::shared_ptr<ASTNumber> array_size) :
        ASTVarDecl(name, !!array_size, array_size)
                                    /* don't move array_size because of
                                     * its use in !!array_size */
    {
    }

    explicit ASTVarDecl(SourceRange name, bool is_array,
                        std::shared_ptr<ASTNumber> array_size) :
        name(name), array_size(std::move(array_size)), is_array(is_array)
    {
    }

    virtual void dump(std::string&, size_t depth);

protected:
    SourceRange name;
    std::shared_ptr<ASTNumber> array_size; //< may be null, even if is_array=true
    bool is_array;
};

/// Node that represents a variable declaration in a function param list.
class ASTParmVarDecl : public ASTVarDecl
{
public:
    explicit ASTParmVarDecl(SourceRange name, bool is_array) :
        ASTVarDecl(name, is_array, nullptr)
    {}

    virtual void dump(std::string&, size_t depth);
};

/// Node that represents a function declaration.
class ASTFunDecl : public ASTDecl
{
public:
    explicit ASTFunDecl(bool is_void_retn, SourceRange name,
                        std::vector<std::shared_ptr<ASTParmVarDecl>> params,
                        std::shared_ptr<ASTCompoundStmt> comp_stmt) :
        comp_stmt(std::move(comp_stmt)),
        params(std::move(params)),
        name(name),
        is_void_retn(is_void_retn)
    {
    }

    virtual void dump(std::string&, size_t depth);

private:
    std::shared_ptr<ASTCompoundStmt> comp_stmt; //< never null
    std::vector<std::shared_ptr<ASTParmVarDecl>> params;
    SourceRange name;
    bool is_void_retn;
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

/// Node of a variable reference in the AST.
class ASTVarRef : public ASTExpr
{
public:
    explicit ASTVarRef(SourceRange varname) :
        varname(varname)
    {
    }

    explicit ASTVarRef(SourceRange varname, std::shared_ptr<ASTExpr> expr) :
        varname(varname), expr(expr)
    {
    }

    virtual void dump(std::string&, size_t depth);

private:
    SourceRange varname;
    std::shared_ptr<ASTExpr> expr;
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
    explicit ASTAssignExpr(std::shared_ptr<ASTVarRef> left,
                           std::shared_ptr<ASTExpr> right) :
        ASTBinaryExpr(std::move(left), std::move(right), Operation::Assign)
    {
    }
};


/// Node of a function call in the AST.
class ASTFunCall : public ASTExpr
{
public:
    virtual void dump(std::string&, size_t depth);
};

class ASTCompoundStmt : public ASTStmt
{
public:
    explicit ASTCompoundStmt(std::vector<std::shared_ptr<ASTVarDecl>> decls,
                             std::vector<std::shared_ptr<ASTStmt>> stms) :
        decls(std::move(decls)), stms(std::move(stms))
    {}
    
    virtual void dump(std::string&, size_t depth);

private:
    std::vector<std::shared_ptr<ASTVarDecl>> decls;
    std::vector<std::shared_ptr<ASTStmt>> stms;
};

}
