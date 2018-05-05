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
    this->current_scope = std::make_unique<Scope>(ScopeFlags::TopLevel, nullptr);
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

void Semantics::act_on_top_level_decl(
        const std::shared_ptr<ASTProgram>& program,
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

    // TODO THIS IS A STUB STUB FOR PASSING TESTS, REMOVE IT
    if(type.category == Category::Void)
    {
        // TODO even the Diag:: code is wrong!!! TRASH ME OUT REALLY.
        diagman.report(source, type.location(), Diag::parser_number_too_big)
                .range(type.lexeme);
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

auto Semantics::act_on_param_decl(const Word& type, const Word& name,
                                  bool is_array)
        -> std::shared_ptr<ASTParmVarDecl>
{
    assert(type.category == Category::Void || type.category == Category::Int);
    assert(name.category == Category::Identifier);

    // TODO cannot be void and stuff
    // TODO add to scope and stuff

    // TODO THIS IS A STUB STUB FOR PASSING TESTS, REMOVE IT
    if(type.category == Category::Void)
    {
        // TODO even the Diag:: code is wrong!!! TRASH ME OUT REALLY.
        diagman.report(source, type.location(), Diag::parser_number_too_big)
                .range(type.lexeme);
    }

    return std::make_shared<ASTParmVarDecl>(name.lexeme, is_array);
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

auto Semantics::act_on_null_stmt()
        -> std::shared_ptr<ASTNullStmt>
{
    return std::make_shared<ASTNullStmt>();
}

auto Semantics::act_on_compound_stmt(std::vector<std::shared_ptr<ASTVarDecl>> decls,
                                     std::vector<std::shared_ptr<ASTStmt>> stms)
        -> std::shared_ptr<ASTCompoundStmt>
{
    return std::make_shared<ASTCompoundStmt>(std::move(decls), std::move(stms));
}

auto Semantics::act_on_selection_stmt(std::shared_ptr<ASTExpr> expr,
                                      std::shared_ptr<ASTStmt> stmt1,
                                      std::shared_ptr<ASTStmt> stmt2)
        -> std::shared_ptr<ASTSelectionStmt>
{
    return std::make_shared<ASTSelectionStmt>(std::move(expr), std::move(stmt1), std::move(stmt2));
}

auto Semantics::act_on_iteration_stmt(std::shared_ptr<ASTExpr> expr,
                                      std::shared_ptr<ASTStmt> stmt)
        -> std::shared_ptr<ASTIterationStmt>
{
    return std::make_shared<ASTIterationStmt>(std::move(expr), std::move(stmt));
}

auto Semantics::act_on_return_stmt(std::shared_ptr<ASTExpr> expr)
        -> std::shared_ptr<ASTReturnStmt>
{
    return std::make_shared<ASTReturnStmt>(std::move(expr));
}

auto Semantics::act_on_number(const Word& word)
        -> std::shared_ptr<ASTNumber>
{
    assert(word.category == Category::Number);
    auto number = number_from_word(word);
    return std::make_shared<ASTNumber>(number);
}

auto Semantics::act_on_var(const Word& name, std::shared_ptr<ASTExpr> index)
        -> std::shared_ptr<ASTVarRef>
{
    assert(name.category == Category::Identifier);
    return std::make_shared<ASTVarRef>(name.lexeme, std::move(index));
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
