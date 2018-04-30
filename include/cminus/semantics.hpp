#pragma once
#include <cminus/ast.hpp>
#include <cminus/diagnostics.hpp>
#include <cminus/sourceman.hpp>
#include <unordered_map>

namespace cminus
{
class Scope
{
public:
    auto add_decl(SourceRange name, std::shared_ptr<ASTDecl> decl)
            -> std::pair<std::shared_ptr<ASTDecl>, bool>
    {
        auto [it, inserted] = symbols.emplace(std::move(name), std::move(decl));
        return std::pair{it->second, inserted};
    }

    // TODO create scopes (in Semantics)

private:
    std::unique_ptr<Scope> parent_scope;
    std::unordered_map<SourceRange, std::shared_ptr<ASTDecl>> symbols;
};

/// The semantic analyzer performs context-sensitive analysis, type-checking,
/// and AST building. It is driven by actions called from within the parser.
class Semantics
{
public:
    explicit Semantics(const SourceFile& source,
                       DiagnosticManager& diagman);

    /// Acts once the parser begins parsing.
    auto act_on_program_start() -> std::shared_ptr<ASTProgram>;

    /// Acts once the parser finishes parsing.
    auto act_on_program_end(std::shared_ptr<ASTProgram> program)
            -> std::shared_ptr<ASTProgram>;

    /// Acts on a program-level declaration.
    // TODO remove
    void act_on_decl(const std::shared_ptr<ASTProgram>& program,
                     std::shared_ptr<ASTDecl> decl);

    /// Acts on a declaration of a new variable.
    auto act_on_var_decl(const Word& type, const Word& name,
                         std::shared_ptr<ASTNumber> array_size)
            -> std::shared_ptr<ASTVarDecl>;

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

    /// Converts a word into a number.
    int32_t number_from_word(const Word& word);

private:
    const SourceFile& source;
    DiagnosticManager& diagman;
    std::unique_ptr<Scope> current_scope;
};
}
