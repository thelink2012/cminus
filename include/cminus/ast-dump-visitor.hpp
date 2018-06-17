#pragma once
#include <cminus/ast-visitor.hpp>

namespace cminus
{
class ASTDumpVisitor : public ASTVisitor
{
public:
    explicit ASTDumpVisitor(std::string& dest) :
        dest(dest)
    {
    }

    void visit_program(ASTProgram& program) override;

    void visit_var_decl(ASTVarDecl& decl) override;
    void visit_parm_decl(ASTParmVarDecl& decl) override;
    void visit_fun_decl(ASTFunDecl& decl) override;

    void visit_null_stmt(ASTNullStmt& stmt) override;
    void visit_compound_stmt(ASTCompoundStmt& stmt) override;
    void visit_selection_stmt(ASTSelectionStmt& stmt) override;
    void visit_iteration_stmt(ASTIterationStmt& stmt) override;
    void visit_return_stmt(ASTReturnStmt& stmt) override;

    void visit_number_expr(ASTNumber& expr) override;
    void visit_var_expr(ASTVarRef& expr) override;
    void visit_call_expr(ASTFunCall& expr) override;
    void visit_binary_expr(ASTBinaryExpr& expr) override;

    void visit_type(ExprType type) override;
    void visit_name(SourceRange name) override;

private:
    auto operation(ASTBinaryExpr::Operation op) -> const char*;
    void newline(size_t depth);

private:
    std::string& dest;
    size_t depth = 0;
};
}
