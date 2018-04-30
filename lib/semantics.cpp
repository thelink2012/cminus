#include <cminus/semantics.hpp>

// NOTE in parser.cpp we assume sema never fails to return a valid shared_ptr!!
// if we change this assumption, please review parser.cpp

namespace cminus
{
Semantics::Semantics(const SourceFile& source_a,
                     DiagnosticManager& diagman_a) :
    source(source_a),
    diagman(diagman_a)
{
    this->current_scope = std::make_unique<Scope>();
}

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

auto Semantics::act_on_var_decl(const Word& type, const Word& name,
                                std::shared_ptr<ASTNumber> array_size)
        -> std::shared_ptr<ASTVarDecl>
{
    assert(type.category == Category::Void || type.category == Category::Int);
    assert(name.category == Category::Identifier);

    // TODO semantically a var decl cannot be void

    auto new_decl = std::make_shared<ASTVarDecl>(name.lexeme, std::move(array_size));

    auto [decl, inserted] = current_scope->add_decl(name.lexeme, new_decl);
    if(!inserted)
    {
        // TODO this is a semantic error (redecl)
    }

    // Return the new declaration no matter what.
    return new_decl;
}

auto Semantics::act_on_fun_decl(const Word& retn_type, const Word& name,
                                std::vector<std::shared_ptr<ASTParmVarDecl>> params,
                                std::shared_ptr<ASTCompoundStmt> comp_stmt)
        -> std::shared_ptr<ASTFunDecl>
{
    assert(retn_type.category == Category::Void 
            || retn_type.category == Category::Int);
    assert(name.category == Category::Identifier);

    auto is_void = (retn_type.category == Category::Void);

    // TODO add decl to scope and all that cheese (semantics)

    return std::make_shared<ASTFunDecl>(is_void, name.lexeme,
                                        std::move(params),
                                        std::move(comp_stmt));
}

auto Semantics::act_on_assign(std::shared_ptr<ASTVarRef> lhs,
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
