#include <cminus/ast-visitor.hpp>

namespace cminus
{
void ASTVisitor::walk_program(ASTProgram& program)
{
    for(auto it = program.decl_begin(); it != program.decl_end(); ++it)
        visit_decl(**it);
}

void ASTVisitor::walk_decl(ASTDecl& decl)
{
    switch(decl.decl_kind())
    {
        case DeclKind::VarDecl:
            visit_var_decl(*decl.as_var_decl());
            break;
        case DeclKind::ParmVarDecl:
            visit_parm_decl(*decl.as_parm_var_decl());
            break;
        case DeclKind::FunDecl:
            visit_fun_decl(*decl.as_fun_decl());
            break;
    }
}

void ASTVisitor::walk_var_decl(ASTVarDecl& var_decl)
{
    visit_type(var_decl.type());
    visit_name(var_decl.get_name());
    if(auto size = var_decl.get_array_size())
        visit_number_expr(*size);
}

void ASTVisitor::walk_parm_decl(ASTParmVarDecl& parm_decl)
{
    visit_type(parm_decl.type());
    visit_name(parm_decl.get_name());
}

void ASTVisitor::walk_fun_decl(ASTFunDecl& fun_decl)
{
    visit_type(fun_decl.type());
    for(auto it = fun_decl.parm_begin(); it != fun_decl.parm_end(); ++it)
        visit_decl(**it);
    if(auto body = fun_decl.get_body())
        visit_stmt(*body);
}

void ASTVisitor::walk_stmt(ASTStmt& stmt)
{
    switch(stmt.stmt_kind())
    {
        case StmtKind::NullStmt:
            visit_null_stmt(*stmt.as_null_stmt());
            break;
        case StmtKind::ExprStmt:
            visit_expr_stmt(*stmt.as_expr_stmt());
            break;
        case StmtKind::CompoundStmt:
            visit_compound_stmt(*stmt.as_compound_stmt());
            break;
        case StmtKind::SelectionStmt:
            visit_selection_stmt(*stmt.as_selection_stmt());
            break;
        case StmtKind::IterationStmt:
            visit_iteration_stmt(*stmt.as_iteration_stmt());
            break;
        case StmtKind::ReturnStmt:
            visit_return_stmt(*stmt.as_return_stmt());
            break;
    }
}

void ASTVisitor::walk_null_stmt(ASTNullStmt&)
{
}

void ASTVisitor::walk_compound_stmt(ASTCompoundStmt& comp_stmt)
{
    for(auto it = comp_stmt.decl_begin(); it != comp_stmt.decl_end(); ++it)
        visit_decl(**it);
    for(auto it = comp_stmt.stmt_begin(); it != comp_stmt.stmt_end(); ++it)
        visit_stmt(**it);
}

void ASTVisitor::walk_selection_stmt(ASTSelectionStmt& if_stmt)
{
    visit_expr(*if_stmt.get_cond());
    visit_stmt(*if_stmt.get_then());
    if(auto stmt2 = if_stmt.get_else())
        visit_stmt(*stmt2);
}

void ASTVisitor::walk_iteration_stmt(ASTIterationStmt& while_stmt)
{
    visit_expr(*while_stmt.get_cond());
    visit_stmt(*while_stmt.get_body());
}

void ASTVisitor::walk_return_stmt(ASTReturnStmt& retn_stmt)
{
    if(auto expr = retn_stmt.get_expr())
        visit_expr(*expr);
}

void ASTVisitor::walk_expr(ASTExpr& expr)
{
    switch(expr.expr_kind())
    {
        case ExprKind::Number:
            visit_number_expr(*expr.as_number_expr());
            break;
        case ExprKind::VarRef:
            visit_var_expr(*expr.as_var_expr());
            break;
        case ExprKind::FunCall:
            visit_call_expr(*expr.as_call_expr());
            break;
        case ExprKind::BinaryExpr:
        case ExprKind::AssignExpr:
            visit_binary_expr(*expr.as_binary_expr());
            break;
    }
}

void ASTVisitor::walk_number_expr(ASTNumber&)
{
    // nothing to visit
}

void ASTVisitor::walk_var_expr(ASTVarRef& var_ref)
{
    visit_name(var_ref.get_decl()->get_name());
    if(auto expr = var_ref.get_index())
        visit_expr(*expr);
}

void ASTVisitor::walk_call_expr(ASTFunCall& fun_call)
{
    visit_name(fun_call.get_decl()->get_name());
    for(auto it = fun_call.arg_begin(); it != fun_call.arg_end(); ++it)
        visit_expr(**it);
}

void ASTVisitor::walk_binary_expr(ASTBinaryExpr& expr)
{
    visit_expr(*expr.get_left());
    visit_expr(*expr.get_right());
}

void ASTVisitor::walk_type(ExprType)
{
    // nothing to visit
}

void ASTVisitor::walk_name(SourceRange)
{
    // nothing to visit
}
}
