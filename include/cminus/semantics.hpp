#pragma once
#include <cminus/sourceman.hpp>
#include <cminus/diagnostics.hpp>
#include <cminus/ast.hpp>

namespace cminus
{
/// The semantic analyzer performs context-sensitive analysis, type-checking,
/// and AST building. It is driven by actions called from within the parser.
class Semantics
{
public:
    explicit Semantics(const SourceFile& source,
                       DiagnosticManager& diagman) :
        source(source), diagman(diagman)
    {}

    /// Acts once the parser begins parsing.
    auto act_on_program_start() -> std::shared_ptr<ASTProgram>;

    /// Acts once the parser finishes parsing.
    auto act_on_program_end(std::shared_ptr<ASTProgram> program) 
        -> std::shared_ptr<ASTProgram>;

    /// Acts on a program-level declaration.
    void act_on_decl(const std::shared_ptr<ASTProgram>& program,
                     std::shared_ptr<ASTDecl> decl);

    /// Acts on an assignment expression.
    auto act_on_assign(std::shared_ptr<ASTExpr> lhs,
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
};
}
