#include <cminus/semantics.hpp>

// NOTE in parser.cpp we assume sema never fails to return a valid shared_ptr!!
// if we change this assumption, please review parser.cpp

namespace cminus
{
auto Semantics::act_on_program_start() -> std::shared_ptr<ASTProgram>
{
    return std::make_shared<ASTProgram>();
}

auto Semantics::act_on_program_end(std::shared_ptr<ASTProgram> program)
   -> std::shared_ptr<ASTProgram>
{
    return program;
}

void Semantics::act_on_decl(const std::shared_ptr<ASTProgram>& program,
                            std::shared_ptr<ASTDecl> decl)
{
    program->add_decl(std::move(decl));
}

auto Semantics::act_on_assign(std::shared_ptr<ASTExpr> lhs,
                              std::shared_ptr<ASTExpr> rhs)
    -> std::shared_ptr<ASTAssignExpr>
{
    return std::make_shared<ASTAssignExpr>(std::move(lhs), std::move(rhs));
}

auto Semantics::act_on_binary_expr(std::shared_ptr<ASTExpr> lhs,
                                   std::shared_ptr<ASTExpr> rhs,
                                   Category category)
   -> std::shared_ptr<ASTBinaryExpr>
{
    auto type = ASTBinaryExpr::type_from_category(category);
    return std::make_shared<ASTBinaryExpr>(std::move(lhs), std::move(rhs), type);
}

auto Semantics::act_on_number(const Word& word)
   -> std::shared_ptr<ASTNumber>
{
    auto number = number_from_word(word);
    return std::make_shared<ASTNumber>(number);
}

auto Semantics::number_from_word(const Word& word) -> int32_t
{
    assert(word.category == Category::Number);
    try
    {
        // TODO use std::from_chars once it's available in libstdc++
        std::string lexeme(word.lexeme.begin(), word.lexeme.size());
        return std::stoi(lexeme, nullptr, 10);
    }
    catch(const std::out_of_range&)
    {
        diagman.report(source, word.location(), Diag::parser_number_too_big)
                .range(word.lexeme);
        return 0;
    }
}
}
