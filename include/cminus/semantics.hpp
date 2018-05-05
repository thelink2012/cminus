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

class Scope
{
public:
    explicit Scope(ScopeFlags flags, std::unique_ptr<Scope> parent_a) :
        parent_scope(std::move(parent_a)), flags(flags)
    {
        assert(!!(flags & ScopeFlags::FunScope) ? !!(flags & ScopeFlags::CompoundStmt) : true);
    }

    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

    auto detach() -> std::unique_ptr<Scope>
    {
        auto prev = std::move(this->parent_scope);
        return prev;
    }

    auto lookup_local(SourceRange name) const -> std::shared_ptr<ASTDecl>
    {
        auto it = symbols.find(name);
        if(it == symbols.end())
            return nullptr;
        return it->second;
    }

    auto add_decl(SourceRange name, std::shared_ptr<ASTDecl> decl)
            -> std::pair<std::shared_ptr<ASTDecl>, bool>
    {
        // If the parent scope is the function parameters scope, look
        // for this name there. This is considered a redeclaration.
        if(parent_scope && parent_scope->is_params_scope())
        {
            if(auto decl = parent_scope->lookup_local(name))
                return std::pair{decl, false};
        }

        auto [it, inserted] = symbols.emplace(std::move(name), std::move(decl));
        return std::pair{it->second, inserted};
    }

    bool is_params_scope() const { return !!(flags & ScopeFlags::FunParamsScope); }

    // TODO create scopes (in Semantics)

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
    explicit Semantics(const SourceFile& source,
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

    /// Acts on the declaration of a new function.
    auto act_on_fun_decl(const Word& retn_type, const Word& name,
                         std::vector<std::shared_ptr<ASTParmVarDecl>> params,
                         std::shared_ptr<ASTCompoundStmt> comp_stmt)
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
    auto act_on_selection_stmt(std::shared_ptr<ASTExpr> expr,
                               std::shared_ptr<ASTStmt> stmt)
            -> std::shared_ptr<ASTSelectionStmt>;

    /// Acts on a selection statement.
    auto act_on_selection_stmt(std::shared_ptr<ASTExpr> expr,
                               std::shared_ptr<ASTStmt> stmt1,
                               std::shared_ptr<ASTStmt> stmt2)
            -> std::shared_ptr<ASTSelectionStmt>;

    /// Acts on an iteration statement.
    auto act_on_iteration_stmt(std::shared_ptr<ASTExpr> expr,
                               std::shared_ptr<ASTStmt> stmt)
            -> std::shared_ptr<ASTIterationStmt>;

    /// Acts on a return statement.
    auto act_on_return_stmt() -> std::shared_ptr<ASTReturnStmt>;

    /// Acts on a return statement.
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

    /// Converts a word into a number.
    int32_t number_from_word(const Word& word);

    void enter_scope(ScopeFlags flags)
    {
        auto old_scope = std::move(current_scope);
        auto new_scope = std::make_unique<Scope>(flags, std::move(old_scope));
        current_scope = std::move(new_scope);
    }

    void leave_scope()
    {
        current_scope = current_scope->detach();
        assert(current_scope != nullptr);
    }

    Scope& get_scope()
    {
        assert(current_scope != nullptr);
        return *current_scope;
    }

private:
    const SourceFile& source;
    DiagnosticManager& diagman;
    std::unique_ptr<Scope> current_scope;
};

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
