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
class ASTVarDecl;
class ASTParmVarDecl;
class ASTBinaryExpr;
class ASTAssignExpr;
class ASTNumber;
class ASTVarRef;
class ASTNullStmt;
class ASTCompoundStmt;
class ASTSelectionStmt;
class ASTIterationStmt;
class ASTReturnStmt;
class ASTFunCall;

enum class ExprType
{
    Array,
    Int,
    Void
};

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

    /// \returns a static pointer cast to T.
    template<typename T>
    auto cast() -> std::shared_ptr<T>
    {
        return std::static_pointer_cast<T>(shared_from_this());
    }

    /// \returns a static pointer cast to T.
    template<typename T>
    auto cast() const -> std::shared_ptr<const T>
    {
        return std::static_pointer_cast<T>(shared_from_this());
    }
};

/// Base of any declaration node.
class ASTDecl : public AST
{
public:
    virtual auto as_fun_decl() -> std::shared_ptr<ASTFunDecl>
    {
        return nullptr;
    }

    virtual auto as_var_decl() -> std::shared_ptr<ASTVarDecl>
    {
        return nullptr;
    }
};

// Base of any statement node.
class ASTStmt : public AST
{
};

/// Base of any expression node.
class ASTExpr : public ASTStmt
{
public:
    virtual auto type() -> ExprType = 0;
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

    /// Get the last declaration.
    auto get_last_decl() -> std::shared_ptr<ASTDecl>
    {
        if(!decls.size())
            return nullptr;
        return decls.back();
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

    explicit ASTVarDecl(SourceRange name, bool is_array_,
                        std::shared_ptr<ASTNumber> array_size) :
        name(name),
        array_size(std::move(array_size)), is_array_(is_array_)
    {
    }

    virtual void dump(std::string&, size_t depth);

    virtual auto as_var_decl() -> std::shared_ptr<ASTVarDecl>
    {
        return this->cast<ASTVarDecl>();
    }

    SourceRange get_name() { return name; }

    bool is_array()
    {
        return this->is_array_;
    }

protected:
    SourceRange name;
    std::shared_ptr<ASTNumber> array_size; //< may be null, even if is_array_=true
    bool is_array_;
};

/// Node that represents a variable declaration in a function param list.
class ASTParmVarDecl : public ASTVarDecl
{
public:
    explicit ASTParmVarDecl(SourceRange name, bool is_array_) :
        ASTVarDecl(name, is_array_, nullptr)
    {
    }

    virtual void dump(std::string&, size_t depth);
};

/// Node that represents a function declaration.
class ASTFunDecl : public ASTDecl
{
public:
    explicit ASTFunDecl(bool is_void_retn, SourceRange name) :
        name(name),
        is_void_retn(is_void_retn)
    {
    }

    virtual void dump(std::string&, size_t depth);

    virtual auto as_fun_decl() -> std::shared_ptr<ASTFunDecl>
    {
        return this->cast<ASTFunDecl>();
    }

    SourceRange get_name() { return name; }

    void set_body(std::shared_ptr<ASTCompoundStmt> comp_stmt)
    {
        this->comp_stmt = std::move(comp_stmt);
    }

    void add_param(std::shared_ptr<ASTParmVarDecl> parm)
    {
        this->params.push_back(std::move(parm));
    }

    bool is_void()
    {
        return this->is_void_retn;
    }

    auto get_num_params() -> size_t
    {
        return this->params.size();
    }

    auto get_param(size_t index) -> std::shared_ptr<ASTParmVarDecl>
    {
        return this->params[index];
    }

private:
    std::shared_ptr<ASTCompoundStmt> comp_stmt; //< may be null
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

    virtual auto type() -> ExprType
    {
        return ExprType::Int;
    }

private:
    int32_t value;
};

/// Node of a variable reference in the AST.
class ASTVarRef : public ASTExpr
{
public:
    explicit ASTVarRef(std::shared_ptr<ASTVarDecl> decl) :
        decl(std::move(decl))
    {
    }

    explicit ASTVarRef(std::shared_ptr<ASTVarDecl> decl,
                       std::shared_ptr<ASTExpr> expr) :
        decl(std::move(decl)),
        expr(std::move(expr))
    {
    }

    virtual void dump(std::string&, size_t depth);

    virtual auto type() -> ExprType
    {
        if(expr)
            return ExprType::Int;
        else if(this->decl->is_array())
            return ExprType::Array;
        else
            return ExprType::Int;
    }

private:
    std::shared_ptr<ASTVarDecl> decl;
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

    virtual auto type() -> ExprType
    {
        return ExprType::Int;
    }

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

class ASTNullStmt : public ASTStmt
{
public:
    explicit ASTNullStmt()
    {
    }

    virtual void dump(std::string&, size_t depth);
};

class ASTCompoundStmt : public ASTStmt
{
public:
    explicit ASTCompoundStmt(std::vector<std::shared_ptr<ASTVarDecl>> decls,
                             std::vector<std::shared_ptr<ASTStmt>> stms) :
        decls(std::move(decls)),
        stms(std::move(stms))
    {
    }

    virtual void dump(std::string&, size_t depth);

private:
    std::vector<std::shared_ptr<ASTVarDecl>> decls;
    std::vector<std::shared_ptr<ASTStmt>> stms;
};

class ASTSelectionStmt : public ASTStmt
{
public:
    explicit ASTSelectionStmt(std::shared_ptr<ASTExpr> expr,
                              std::shared_ptr<ASTStmt> stmt1,
                              std::shared_ptr<ASTStmt> stmt2 = nullptr) :
        expr(std::move(expr)),
        stmt1(std::move(stmt1)),
        stmt2(std::move(stmt2))
    {
    }

    virtual void dump(std::string&, size_t depth);

private:
    std::shared_ptr<ASTExpr> expr;
    std::shared_ptr<ASTStmt> stmt1;
    std::shared_ptr<ASTStmt> stmt2; //< may be null for no-else
};

class ASTIterationStmt : public ASTStmt
{
public:
    explicit ASTIterationStmt(std::shared_ptr<ASTExpr> expr,
                              std::shared_ptr<ASTStmt> stmt) :
        expr(std::move(expr)),
        stmt(std::move(stmt))
    {
    }

    virtual void dump(std::string&, size_t depth);

private:
    std::shared_ptr<ASTExpr> expr;
    std::shared_ptr<ASTStmt> stmt;
};

class ASTReturnStmt : public ASTStmt
{
public:
    explicit ASTReturnStmt(std::shared_ptr<ASTExpr> expr = nullptr) :
        expr(std::move(expr))
    {
    }

    virtual void dump(std::string&, size_t depth);

private:
    std::shared_ptr<ASTExpr> expr; //< may be null
};

/// Node of a function call in the AST.
class ASTFunCall : public ASTExpr
{
public:
    explicit ASTFunCall(std::shared_ptr<ASTFunDecl> decl,
                        std::vector<std::shared_ptr<ASTExpr>> args) :
        decl(std::move(decl)),
        args(std::move(args))
    {
    }

    virtual void dump(std::string&, size_t depth);

    virtual auto type() -> ExprType
    {
        if(this->decl->is_void())
            return ExprType::Void;
        else
            return ExprType::Int;
    }

private:
    std::shared_ptr<ASTFunDecl> decl;
    std::vector<std::shared_ptr<ASTExpr>> args;
};
}
