#pragma once
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

/// The typing of a expression.
enum class ExprType
{
    Void,
    Int,
    Array,
};

/// The subclass of a declaration.
enum class DeclKind
{
    VarDecl,
    ParmVarDecl,
    FunDecl,
};

/// The subclass of a statement.
enum class StmtKind
{
    NullStmt,
    ExprStmt,
    CompoundStmt,
    SelectionStmt,
    IterationStmt,
    ReturnStmt,
};

/// The subclass of a expression.
enum class ExprKind
{
    Number,
    VarRef,
    FunCall,
    BinaryExpr,
    AssignExpr,
};

/// Base of any AST node.
class AST : public std::enable_shared_from_this<AST>
{
public:
    virtual ~AST() {}

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
    virtual auto decl_kind() const -> DeclKind = 0;

    virtual auto as_fun_decl() -> std::shared_ptr<ASTFunDecl>
    {
        return nullptr;
    }

    virtual auto as_var_decl() -> std::shared_ptr<ASTVarDecl>
    {
        return nullptr;
    }

    virtual auto as_parm_var_decl() -> std::shared_ptr<ASTParmVarDecl>
    {
        return nullptr;
    }
};

// Base of any statement node.
class ASTStmt : public AST
{
public:
    virtual auto stmt_kind() const -> StmtKind = 0;

    virtual auto as_null_stmt() -> std::shared_ptr<ASTNullStmt>
    {
        return nullptr;
    }

    virtual auto as_expr_stmt() -> std::shared_ptr<ASTExpr>
    {
        return nullptr;
    }

    virtual auto as_compound_stmt() -> std::shared_ptr<ASTCompoundStmt>
    {
        return nullptr;
    }

    virtual auto as_selection_stmt() -> std::shared_ptr<ASTSelectionStmt>
    {
        return nullptr;
    }

    virtual auto as_iteration_stmt() -> std::shared_ptr<ASTIterationStmt>
    {
        return nullptr;
    }

    virtual auto as_return_stmt() -> std::shared_ptr<ASTReturnStmt>
    {
        return nullptr;
    }

    auto as_expr() -> std::shared_ptr<ASTExpr>
    {
        return as_expr_stmt();
    }
};

/// Base of any expression node.
class ASTExpr : public ASTStmt
{
public:
    virtual auto expr_kind() const -> ExprKind = 0;

    virtual auto as_number_expr() -> std::shared_ptr<ASTNumber>
    {
        return nullptr;
    }

    virtual auto as_var_expr() -> std::shared_ptr<ASTVarRef>
    {
        return nullptr;
    }

    virtual auto as_call_expr() -> std::shared_ptr<ASTFunCall>
    {
        return nullptr;
    }

    virtual auto as_binary_expr() -> std::shared_ptr<ASTBinaryExpr>
    {
        return nullptr;
    }

    virtual auto type() const -> ExprType = 0;

    virtual auto source_range() const -> SourceRange = 0;

    auto location() const -> SourceLocation
    {
        return source_range().begin();
    }

    auto stmt_kind() const -> StmtKind override
    {
        return StmtKind::ExprStmt;
    }

    auto as_expr_stmt() -> std::shared_ptr<ASTExpr> override
    {
        return this->cast<ASTExpr>();
    }
};

/// Node that represents an entire program.
class ASTProgram : public AST
{
public:
    explicit ASTProgram()
    {
    }

    /// Adds a new declaration into the program.
    void add_decl(std::shared_ptr<ASTDecl> decl)
    {
        assert(decl != nullptr);
        decls.push_back(std::move(decl));
    }

    auto decl_begin() { return decls.begin(); }
    auto decl_end() { return decls.end(); }

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
    {
    }

    explicit ASTVarDecl(SourceRange name, bool is_array_,
                        std::shared_ptr<ASTNumber> array_size) :
        name(name),
        array_size(std::move(array_size)), is_array_(is_array_)
    {
    }

    auto decl_kind() const -> DeclKind override
    {
        return DeclKind::VarDecl;
    }

    auto as_var_decl() -> std::shared_ptr<ASTVarDecl> override
    {
        return this->cast<ASTVarDecl>();
    }

    auto type() const -> ExprType
    {
        return is_array() ? ExprType::Array : ExprType::Int;
    }

    SourceRange get_name() const { return name; }

    bool is_array() const { return this->is_array_; }

    auto get_array_size() const -> std::shared_ptr<ASTNumber> { return array_size; }

protected:
    SourceRange name;
    std::shared_ptr<ASTNumber> array_size; //< may be null, even if is_array_=true
                                           //< e.g. for function params which are array
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

    auto decl_kind() const -> DeclKind override
    {
        return DeclKind::ParmVarDecl;
    }

    auto as_parm_var_decl() -> std::shared_ptr<ASTParmVarDecl> override
    {
        return this->cast<ASTParmVarDecl>();
    }
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

    auto parm_begin() { return params.begin(); }
    auto parm_end() { return params.end(); }

    auto decl_kind() const -> DeclKind override
    {
        return DeclKind::FunDecl;
    }

    auto as_fun_decl() -> std::shared_ptr<ASTFunDecl> override
    {
        return this->cast<ASTFunDecl>();
    }

    SourceRange get_name() const { return name; }

    auto type() const { return is_void() ? ExprType::Void : ExprType::Int; }

    bool is_void() const { return this->is_void_retn; }

    size_t get_num_params() const { return this->params.size(); }

    auto get_param(size_t index) -> std::shared_ptr<ASTParmVarDecl>
    {
        return this->params[index];
    }

    void set_body(std::shared_ptr<ASTCompoundStmt> comp_stmt)
    {
        this->comp_stmt = std::move(comp_stmt);
    }

    /// \returns the function body or `nullptr` if none.
    auto get_body() -> std::shared_ptr<ASTCompoundStmt>
    {
        return this->comp_stmt;
    }

    void add_param(std::shared_ptr<ASTParmVarDecl> parm)
    {
        this->params.push_back(std::move(parm));
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
    explicit ASTNumber(int32_t number, SourceRange lexeme) :
        loc(lexeme), value(number)
    {
    }

    auto get_value() const -> int32_t { return value; }

    auto expr_kind() const -> ExprKind override
    {
        return ExprKind::Number;
    }

    auto as_number_expr() -> std::shared_ptr<ASTNumber> override
    {
        return this->cast<ASTNumber>();
    }

    auto type() const -> ExprType override
    {
        return ExprType::Int;
    }

    auto source_range() const -> SourceRange override
    {
        return loc;
    }

private:
    SourceRange loc;
    int32_t value;
};

/// Node of a variable reference in the AST.
class ASTVarRef : public ASTExpr
{
public:
    explicit ASTVarRef(std::shared_ptr<ASTVarDecl> decl,
                       std::shared_ptr<ASTExpr> expr,
                       SourceRange loc) :
        decl(std::move(decl)),
        expr(std::move(expr)),
        loc(loc)
    {
    }

    auto type() const -> ExprType override
    {
        if(expr)
            return ExprType::Int;
        else if(this->decl->is_array())
            return ExprType::Array;
        else
            return ExprType::Int;
    }

    auto get_decl() -> std::shared_ptr<ASTVarDecl>
    {
        return decl;
    }

    /// \returns the subscript expression or `nullptr` if none.
    auto get_index() -> std::shared_ptr<ASTExpr>
    {
        return expr;
    }

    auto expr_kind() const -> ExprKind override
    {
        return ExprKind::VarRef;
    }

    auto as_var_expr() -> std::shared_ptr<ASTVarRef> override
    {
        return this->cast<ASTVarRef>();
    }

    auto source_range() const -> SourceRange override
    {
        return loc;
    }

private:
    std::shared_ptr<ASTVarDecl> decl;
    std::shared_ptr<ASTExpr> expr; //< subscript expression, may be null
    SourceRange loc;
};

/// Node of a function call in the AST.
class ASTFunCall : public ASTExpr
{
public:
    explicit ASTFunCall(std::shared_ptr<ASTFunDecl> decl,
                        std::vector<std::shared_ptr<ASTExpr>> args,
                        SourceRange loc) :
        decl(std::move(decl)),
        args(std::move(args)),
        loc(loc)
    {
    }

    auto arg_begin() { return args.begin(); }
    auto arg_end() { return args.end(); }

    auto type() const -> ExprType override
    {
        if(this->decl->is_void())
            return ExprType::Void;
        else
            return ExprType::Int;
    }

    auto get_decl() -> std::shared_ptr<ASTFunDecl>
    {
        return decl;
    }

    auto expr_kind() const -> ExprKind override
    {
        return ExprKind::FunCall;
    }

    auto as_call_expr() -> std::shared_ptr<ASTFunCall> override
    {
        return this->cast<ASTFunCall>();
    }

    auto source_range() const -> SourceRange override
    {
        return loc;
    }

private:
    std::shared_ptr<ASTFunDecl> decl;
    std::vector<std::shared_ptr<ASTExpr>> args;
    SourceRange loc;
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

    auto type() const -> ExprType override
    {
        return ExprType::Int;
    }

    auto get_left() -> std::shared_ptr<ASTExpr> { return left; }
    auto get_right() -> std::shared_ptr<ASTExpr> { return right; }
    auto get_operation() const -> Operation { return op; }

    auto expr_kind() const -> ExprKind override
    {
        return ExprKind::BinaryExpr;
    }

    auto as_binary_expr() -> std::shared_ptr<ASTBinaryExpr> override
    {
        return this->cast<ASTBinaryExpr>();
    }

    auto source_range() const -> SourceRange override
    {
        auto left_loc = left->source_range().begin();
        auto right_loc = right->source_range().end();
        return SourceRange(left_loc, std::distance(left_loc, right_loc));
    }

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

    auto expr_kind() const -> ExprKind override
    {
        return ExprKind::AssignExpr;
    }
};

/// Node for an empty statement in the AST.
class ASTNullStmt : public ASTStmt
{
public:
    auto stmt_kind() const -> StmtKind override
    {
        return StmtKind::NullStmt;
    }

    auto as_null_stmt() -> std::shared_ptr<ASTNullStmt> override
    {
        return this->cast<ASTNullStmt>();
    }
};

// Node for a compound statement in the AST.
class ASTCompoundStmt : public ASTStmt
{
public:
    explicit ASTCompoundStmt(std::vector<std::shared_ptr<ASTVarDecl>> decls,
                             std::vector<std::shared_ptr<ASTStmt>> stms) :
        decls(std::move(decls)),
        stms(std::move(stms))
    {
    }

    auto decl_begin() { return decls.begin(); }
    auto decl_end() { return decls.end(); }

    auto stmt_begin() { return stms.begin(); }
    auto stmt_end() { return stms.end(); }

    auto stmt_kind() const -> StmtKind override
    {
        return StmtKind::CompoundStmt;
    }

    auto as_compound_stmt() -> std::shared_ptr<ASTCompoundStmt> override
    {
        return this->cast<ASTCompoundStmt>();
    }

private:
    std::vector<std::shared_ptr<ASTVarDecl>> decls;
    std::vector<std::shared_ptr<ASTStmt>> stms;
};

// Node for an if statement in the AST.
class ASTSelectionStmt : public ASTStmt
{
public:
    explicit ASTSelectionStmt(std::shared_ptr<ASTExpr> expr,
                              std::shared_ptr<ASTStmt> stmt1,
                              std::shared_ptr<ASTStmt> stmt2) :
        expr(std::move(expr)),
        stmt1(std::move(stmt1)),
        stmt2(std::move(stmt2))
    {
    }

    auto get_cond() -> std::shared_ptr<ASTExpr> { return expr; }
    auto get_then() -> std::shared_ptr<ASTStmt> { return stmt1; }
    auto get_else() -> std::shared_ptr<ASTStmt> { return stmt2; }

    auto stmt_kind() const -> StmtKind override
    {
        return StmtKind::SelectionStmt;
    }

    auto as_selection_stmt() -> std::shared_ptr<ASTSelectionStmt> override
    {
        return this->cast<ASTSelectionStmt>();
    }

private:
    std::shared_ptr<ASTExpr> expr;
    std::shared_ptr<ASTStmt> stmt1;
    std::shared_ptr<ASTStmt> stmt2; //< may be null
};

// Node for a while statement in the AST.
class ASTIterationStmt : public ASTStmt
{
public:
    explicit ASTIterationStmt(std::shared_ptr<ASTExpr> expr,
                              std::shared_ptr<ASTStmt> stmt) :
        expr(std::move(expr)),
        stmt(std::move(stmt))
    {
    }

    auto get_cond() -> std::shared_ptr<ASTExpr> { return expr; }
    auto get_body() -> std::shared_ptr<ASTStmt> { return stmt; }

    auto stmt_kind() const -> StmtKind override
    {
        return StmtKind::IterationStmt;
    }

    auto as_iteration_stmt() -> std::shared_ptr<ASTIterationStmt> override
    {
        return this->cast<ASTIterationStmt>();
    }

private:
    std::shared_ptr<ASTExpr> expr;
    std::shared_ptr<ASTStmt> stmt;
};

// Node for a return statement in the AST.
class ASTReturnStmt : public ASTStmt
{
public:
    explicit ASTReturnStmt(std::shared_ptr<ASTExpr> expr) :
        expr(std::move(expr))
    {
    }

    /// \returns the return expression or `nullptr` if none.
    auto get_expr() -> std::shared_ptr<ASTExpr> { return expr; }

    auto stmt_kind() const -> StmtKind override
    {
        return StmtKind::ReturnStmt;
    }

    auto as_return_stmt() -> std::shared_ptr<ASTReturnStmt> override
    {
        return this->cast<ASTReturnStmt>();
    }

private:
    std::shared_ptr<ASTExpr> expr; //< may be null
};
}
