#include <cminus/ast-dump-visitor.hpp>

namespace cminus
{
void ASTDumpVisitor::visit_program(ASTProgram& program)
{
}

void ASTDumpVisitor::visit_var_decl(ASTVarDecl& decl)
{
}

void ASTDumpVisitor::visit_parm_decl(ASTParmVarDecl& decl)
{
}

void ASTDumpVisitor::visit_fun_decl(ASTFunDecl& decl)
{
}

void ASTDumpVisitor::visit_null_stmt(ASTNullStmt&)
{
}

void ASTDumpVisitor::visit_compound_stmt(ASTCompoundStmt& comp_stmt)
{
}

void ASTDumpVisitor::visit_selection_stmt(ASTSelectionStmt& if_stmt)
{
}

void ASTDumpVisitor::visit_iteration_stmt(ASTIterationStmt& while_stmt)
{
}

void ASTDumpVisitor::visit_return_stmt(ASTReturnStmt& retn_stmt)
{
}

void ASTDumpVisitor::visit_binary_expr(ASTBinaryExpr& expr)
{
}

void ASTDumpVisitor::visit_number_expr(ASTNumber& num)
{
}

void ASTDumpVisitor::visit_var_expr(ASTVarRef& var)
{
}

void ASTDumpVisitor::visit_call_expr(ASTFunCall& fun_call)
{
}

void ASTDumpVisitor::visit_type(ExprType type)
{
}

void ASTDumpVisitor::visit_name(SourceRange name)
{
}
}
