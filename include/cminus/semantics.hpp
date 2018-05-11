#pragma once
#include <cminus/ast.hpp>
#include <cminus/diagnostics.hpp>
#include <cminus/sourceman.hpp>
#include <unordered_map>

namespace cminus
{
enum class ScopeFlags : uint32_t
{
    /// This is the top-level program scope.
    TopLevel = (1 << 0),

    /// The scope of parameters of a function definition.
    /// This scope is right before the compound statement scope of a function.
    FunParamsScope = (1 << 1),

    /// This is the scope of the compound statement following the
    /// function declaration. Implies `ScopeFlags::CompoundStmt`.
    FunScope = (1 << 2),

    /// This is a compound statement scope.
    CompoundStmt = (1 << 3),
};

constexpr ScopeFlags operator|(ScopeFlags lhs, ScopeFlags rhs)
{
    return static_cast<ScopeFlags>(
            static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr ScopeFlags operator&(ScopeFlags lhs, ScopeFlags rhs)
{
    return static_cast<ScopeFlags>(
            static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr bool operator!(ScopeFlags value)
{
    return !(static_cast<uint32_t>(value));
}

/// This stores scope information, including a symbol table.
class Scope
{
public:
    explicit Scope(ScopeFlags flags, std::unique_ptr<Scope> parent_a) :
        parent_scope(std::move(parent_a)), flags(flags)
    {
        assert(!!(flags & ScopeFlags::FunScope) ? 
                !!(flags & ScopeFlags::CompoundStmt) : true);
    }

    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

    /// Detaches and returns the parent scope.
    auto detach() -> std::unique_ptr<Scope>;

    /// Performs a symbol lookup.
    ///
    /// \returns the symbol information or `nullptr` if no such symbol exists.
    auto lookup(SourceRange name) const -> std::shared_ptr<ASTDecl>;

    /// Performs a symbol lookup exclusively on this scope.
    /// 
    /// In other words, the lookup request is not propagated to the parent scope.
    auto lookup_exclusive(SourceRange name) const -> std::shared_ptr<ASTDecl>;

    /// Inserts a new symbol into this scope.
    /// 
    /// If this is a redeclaration, no changes are made to the symbol table.
    ///
    /// \returns a pair consisting of a pointer to the inserted symbol (or to the
    /// symbol that prevented the insertion) and a bool denoting whether the
    /// insertion took place.
    auto insert(SourceRange name, std::shared_ptr<ASTDecl> decl)
            -> std::pair<std::shared_ptr<ASTDecl>, bool>;

    /// Checks whether this is the scope of function parameters.
    bool is_params_scope() const { return !!(flags & ScopeFlags::FunParamsScope); }

private:
    std::unique_ptr<Scope> parent_scope;
    std::unordered_map<SourceRange, std::shared_ptr<ASTDecl>> symbols;
    ScopeFlags flags;
};

/// The semantic analyzer performs context-sensitive analysis, type-checking,
/// and AST building. It is driven by actions called from within the parser.
class Semantics
{
public:
    explicit Semantics(SourceFile& source,
                       DiagnosticManager& diagman);

    Semantics(const Semantics&) = delete;
    Semantics& operator=(const Semantics&) = delete;

    /// Acts once the parser begins parsing.
    auto act_on_program_start() -> std::shared_ptr<ASTProgram>;

    /// Acts once the parser finishes parsing.
    auto act_on_program_end(std::shared_ptr<ASTProgram> program)
            -> std::shared_ptr<ASTProgram>;

    /// Acts on a program-level declaration.
    void act_on_top_level_decl(const std::shared_ptr<ASTProgram>& program,
                               std::shared_ptr<ASTDecl> decl);

    /// Acts on the declaration of a new variable.
    auto act_on_var_decl(const Word& type, const Word& name,
                         std::shared_ptr<ASTNumber> array_size)
            -> std::shared_ptr<ASTVarDecl>;

    /// Acts on the declaration of a new function, but before its parameters
    /// and body are parsed.
    auto act_on_fun_decl_start(const Word& retn_type, const Word& name)
            -> std::shared_ptr<ASTFunDecl>;

    /// Acts on the declaration of a new function once its parameters and body
    /// were parsed.
    auto act_on_fun_decl_end(std::shared_ptr<ASTFunDecl>)
            -> std::shared_ptr<ASTFunDecl>;

    /// Acts on the declaration of a parameter.
    auto act_on_param_decl(const Word& type, const Word& name, bool is_array)
            -> std::shared_ptr<ASTParmVarDecl>;

    /// Acts on a null statement.
    auto act_on_null_stmt() -> std::shared_ptr<ASTNullStmt>;

    /// Acts on a compound statement.
    auto act_on_compound_stmt(std::vector<std::shared_ptr<ASTVarDecl>> decls,
                              std::vector<std::shared_ptr<ASTStmt>> stms)
            -> std::shared_ptr<ASTCompoundStmt>;

    /// Acts on a selection statement.
    ///
    /// The `stmt2` may be `nullptr` for no else statement.
    auto act_on_selection_stmt(std::shared_ptr<ASTExpr> expr,
                               std::shared_ptr<ASTStmt> stmt1,
                               std::shared_ptr<ASTStmt> stmt2)
            -> std::shared_ptr<ASTSelectionStmt>;

    /// Acts on an iteration statement.
    auto act_on_iteration_stmt(std::shared_ptr<ASTExpr> expr,
                               std::shared_ptr<ASTStmt> stmt)
            -> std::shared_ptr<ASTIterationStmt>;

    /// Acts on a return statement.
    ///
    /// The returned `expr` may be `nullptr` for no expression to return.
    auto act_on_return_stmt(std::shared_ptr<ASTExpr> expr)
            -> std::shared_ptr<ASTReturnStmt>;

    /// Acts on an assignment expression.
    auto act_on_assign(std::shared_ptr<ASTVarRef> lhs,
                       std::shared_ptr<ASTExpr> rhs)
            -> std::shared_ptr<ASTAssignExpr>;

    /// Acts on a binary expression.
    auto act_on_binary_expr(std::shared_ptr<ASTExpr> lhs,
                            std::shared_ptr<ASTExpr> rhs,
                            Category category)
            -> std::shared_ptr<ASTBinaryExpr>;

    /// Acts on a number.
    auto act_on_number(const Word& word)
            -> std::shared_ptr<ASTNumber>;

    /// Acts on reference to a variable.
    auto act_on_var(const Word& name, std::shared_ptr<ASTExpr> index)
            -> std::shared_ptr<ASTVarRef>;

    /// Acts on a function call.
    auto act_on_call(const Word& name,
                     std::vector<std::shared_ptr<ASTExpr>> args)
            -> std::shared_ptr<ASTFunCall>;

    /// Converts a word into a number.
    int32_t number_from_word(const Word& word);

    /// Gets the current scope.
    Scope& get_scope();

protected:
    friend class ParseScope;

    /// Enters a new scope.
    /// 
    /// \note use `ParseScope` instead of this method.
    void enter_scope(ScopeFlags flags);

    /// Leaves the previous scope.
    ///
    /// \note use `ParseScope` instead of this method.
    void leave_scope();

private:
    auto make_builtin(Category retn_type, 
                      std::string name,
                      std::vector<std::string> params)
        -> std::shared_ptr<ASTFunDecl>;

private:
    SourceFile& source;
    DiagnosticManager& diagman;
    std::unique_ptr<Scope> current_scope;

    std::shared_ptr<ASTFunDecl> fun_println;
    std::shared_ptr<ASTFunDecl> fun_input;
};

/// This object retains the ownership of a semantic scope.
///
/// Once this object is destroyed, the owned scope gets detached from
/// the semantic context as well.
class ParseScope
{
public:
    explicit ParseScope(Semantics& sema, ScopeFlags flags) :
        sema(sema)
    {
        this->sema.enter_scope(flags);
        this->handle = std::addressof(sema.get_scope());
    }

    ~ParseScope()
    {
        auto current_handle = std::addressof(sema.get_scope());
        assert(current_handle == handle);
        this->sema.leave_scope();
    }

    ParseScope(const ParseScope&) = delete;
    ParseScope(ParseScope&&) = delete;

    ParseScope& operator=(const ParseScope&) = delete;
    ParseScope& operator=(ParseScope&&) = delete;

private:
    Scope* handle;
    Semantics& sema;
};
}
