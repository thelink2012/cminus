#include <cminus/semantics.hpp>

namespace cminus
{
auto Scope::detach() -> std::unique_ptr<Scope>
{
    auto prev = std::move(this->parent_scope);
    return prev;
}

auto Scope::lookup_exclusive(SourceRange name) const -> std::shared_ptr<ASTDecl>
{
    auto it = symbols.find(name);
    if(it == symbols.end())
        return nullptr;
    return it->second;
}

auto Scope::lookup(SourceRange name) const -> std::shared_ptr<ASTDecl>
{
    auto decl = lookup_exclusive(name);
    if(decl == nullptr && parent_scope)
        decl = parent_scope->lookup(name);
    return decl;
}

auto Scope::insert(SourceRange name, std::shared_ptr<ASTDecl> decl)
        -> std::pair<std::shared_ptr<ASTDecl>, bool>
{
    // If the parent scope is the function parameters scope, lookup
    // this name there. This would be considered a redeclaration.
    if(parent_scope && parent_scope->is_params_scope())
    {
        if(auto decl = parent_scope->lookup_exclusive(name))
            return std::pair{decl, false};
    }

    auto [it, inserted] = symbols.emplace(std::move(name), std::move(decl));
    return std::pair{it->second, inserted};
}

void Semantics::enter_scope(ScopeFlags flags)
{
    auto old_scope = std::move(current_scope);
    auto new_scope = std::make_unique<Scope>(flags, std::move(old_scope));
    current_scope = std::move(new_scope);
}

void Semantics::leave_scope()
{
    current_scope = current_scope->detach();
    assert(current_scope != nullptr);
}

auto Semantics::get_scope() -> Scope&
{
    assert(current_scope != nullptr);
    return *current_scope;
}

Semantics::Semantics(SourceFile& source_a,
                     DiagnosticManager& diagman_a) :
    source(source_a),
    diagman(diagman_a)
{
    current_scope = std::make_unique<Scope>(ScopeFlags::TopLevel, nullptr);

    fun_println = make_builtin(Category::Void, "println", {"value"});
    fun_input = make_builtin(Category::Int, "input", {});
}

auto Semantics::make_builtin(Category retn_type,
                             std::string name_a,
                             std::vector<std::string> params)
        -> std::shared_ptr<ASTFunDecl>
{
    assert(retn_type == Category::Void
           || retn_type == Category::Int);

    auto is_void = (retn_type == Category::Void);
    auto name = source.make_source_range(std::move(name_a));
    auto fun_decl = std::make_shared<ASTFunDecl>(is_void, name);

    for(auto&& parm_name_owned : params)
    {
        auto parm_name = source.make_source_range(std::move(parm_name_owned));
        fun_decl->add_param(std::make_shared<ASTParmVarDecl>(parm_name, false));
    }

    auto [decl, inserted] = current_scope->insert(name, fun_decl);
    assert(inserted);

    return fun_decl;
}

auto Semantics::act_on_program_start() -> std::shared_ptr<ASTProgram>
{
    return std::make_shared<ASTProgram>();
}

auto Semantics::act_on_program_end(std::shared_ptr<ASTProgram> program)
        -> std::shared_ptr<ASTProgram>
{
    auto decl = program->get_last_decl();
    if(!decl)
    {
        //TODO diagman
        return nullptr;
    }

    auto fun_decl = decl->as_fun_decl();
    if(!fun_decl)
    {
        //TODO diagman
        return nullptr;
    }
    if(!fun_decl->is_void() || fun_decl->get_name()!="main" || fun_decl->get_num_params())
    {
        //TODO diagman
        return nullptr;
    }

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

    auto new_decl = std::make_shared<ASTVarDecl>(name.lexeme, std::move(array_size));

    auto [decl, inserted] = current_scope->insert(name.lexeme, new_decl);
    if(!inserted)
    {
        diagman.report(source, name.location(),
                       Diag::sema_redefinition, name.lexeme)
                .range(name.lexeme);
    }

    if(type.category == Category::Void)
    {
        diagman.report(source, type.location(), Diag::sema_var_cannot_be_void)
                .range(type.lexeme);
    }

    return new_decl;
}

auto Semantics::act_on_fun_decl_start(const Word& retn_type, const Word& name)
        -> std::shared_ptr<ASTFunDecl>
{
    assert(retn_type.category == Category::Void
           || retn_type.category == Category::Int);
    assert(name.category == Category::Identifier);

    auto is_void = (retn_type.category == Category::Void);

    auto new_decl = std::make_shared<ASTFunDecl>(is_void, name.lexeme);

    auto [decl, inserted] = current_scope->insert(name.lexeme, new_decl);
    if(!inserted)
    {
        diagman.report(source, name.location(),
                       Diag::sema_redefinition, name.lexeme)
                .range(name.lexeme);
    }

    return new_decl;
}

auto Semantics::act_on_fun_decl_end(std::shared_ptr<ASTFunDecl> decl)
        -> std::shared_ptr<ASTFunDecl>
{
    return decl;
}

auto Semantics::act_on_param_decl(const Word& type, const Word& name,
                                  bool is_array)
        -> std::shared_ptr<ASTParmVarDecl>
{
    assert(type.category == Category::Void || type.category == Category::Int);
    assert(name.category == Category::Identifier);

    auto new_decl = std::make_shared<ASTParmVarDecl>(name.lexeme, is_array);

    auto [decl, inserted] = current_scope->insert(name.lexeme, new_decl);
    if(!inserted)
    {
        diagman.report(source, name.location(),
                       Diag::sema_redefinition, name.lexeme)
                .range(name.lexeme);
    }

    if(type.category == Category::Void)
    {
        diagman.report(source, type.location(), Diag::sema_var_cannot_be_void)
                .range(type.lexeme);
    }

    return new_decl;
}

auto Semantics::act_on_assign(std::shared_ptr<ASTVarRef> lhs,
                              std::shared_ptr<ASTExpr> rhs)
        -> std::shared_ptr<ASTAssignExpr>
{
    // TODO type check lhs and rhs.
    return std::make_shared<ASTAssignExpr>(std::move(lhs), std::move(rhs));
}

auto Semantics::act_on_binary_expr(std::shared_ptr<ASTExpr> lhs,
                                   std::shared_ptr<ASTExpr> rhs,
                                   Category category)
        -> std::shared_ptr<ASTBinaryExpr>
{
    // TODO type check lhs and rhs.
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
    // TODO type check expr
    return std::make_shared<ASTSelectionStmt>(std::move(expr), std::move(stmt1), std::move(stmt2));
}

auto Semantics::act_on_iteration_stmt(std::shared_ptr<ASTExpr> expr,
                                      std::shared_ptr<ASTStmt> stmt)
        -> std::shared_ptr<ASTIterationStmt>
{
    // TODO type check expr
    return std::make_shared<ASTIterationStmt>(std::move(expr), std::move(stmt));
}

auto Semantics::act_on_return_stmt(std::shared_ptr<ASTExpr> expr)
        -> std::shared_ptr<ASTReturnStmt>
{
    // TODO type check expr
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

    auto decl = current_scope->lookup(name.lexeme);
    if(!decl)
    {
        diagman.report(source, name.location(),
                       Diag::sema_undeclared_identifier, name.lexeme)
                .range(name.lexeme);
        return nullptr;
    }

    auto var_decl = decl->as_var_decl();
    if(!var_decl)
    {
        diagman.report(source, name.location(), Diag::sema_var_is_not_var)
                .range(name.lexeme);
        return nullptr;
    }

    // TODO type check index

    return std::make_shared<ASTVarRef>(std::move(var_decl), std::move(index));
}

auto Semantics::act_on_call(const Word& name,
                            std::vector<std::shared_ptr<ASTExpr>> args)
        -> std::shared_ptr<ASTFunCall>
{
    assert(name.category == Category::Identifier);

    auto decl = current_scope->lookup(name.lexeme);
    if(!decl)
    {
        diagman.report(source, name.location(),
                       Diag::sema_undeclared_identifier, name.lexeme)
                .range(name.lexeme);
        return nullptr;
    }

    auto fun_decl = decl->as_fun_decl();
    if(!fun_decl)
    {
        diagman.report(source, name.location(), Diag::sema_fun_is_not_fun)
                .range(name.lexeme);
        return nullptr;
    }

    // TODO type check arguments with fun params.
    // TODO match arg count with param count.

    return std::make_shared<ASTFunCall>(std::move(fun_decl), std::move(args));
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
