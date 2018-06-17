#pragma once
#include <cminus/ast.hpp>

namespace cminus
{
/// A class that traverses the abstract syntax tree.
///
/// Each visit method may be overriden to either change the behaviour of the
/// traversal or execute an operation on the node.
///
/// The default implementation of each `visit_*` method recursively visits the
/// childrens of the node by calling its respective `walk_*` method.
///
/// For instance, the default implementation of `visit_selection_stmt` simply
/// calls `walk_selection_stmt` which traverses on its childrens.
///
/// Each node of the tree is guaranted to be visited exacly once.
class ASTVisitor
{
public:
    virtual void visit_program(ASTProgram& program) { walk_program(program); }

    virtual void visit_var_decl(ASTVarDecl& decl) { walk_var_decl(decl); }
    virtual void visit_parm_decl(ASTParmVarDecl& decl) { walk_parm_decl(decl); }
    virtual void visit_fun_decl(ASTFunDecl& decl) { walk_fun_decl(decl); }

    virtual void visit_null_stmt(ASTNullStmt& stmt) { walk_null_stmt(stmt); }
    virtual void visit_compound_stmt(ASTCompoundStmt& stmt) { walk_compound_stmt(stmt); }
    virtual void visit_selection_stmt(ASTSelectionStmt& stmt) { walk_selection_stmt(stmt); }
    virtual void visit_iteration_stmt(ASTIterationStmt& stmt) { walk_iteration_stmt(stmt); }
    virtual void visit_return_stmt(ASTReturnStmt& stmt) { walk_return_stmt(stmt); }

    virtual void visit_number_expr(ASTNumber& expr) { walk_number_expr(expr); }
    virtual void visit_var_expr(ASTVarRef& expr) { walk_var_expr(expr); }
    virtual void visit_call_expr(ASTFunCall& expr) { walk_call_expr(expr); }
    virtual void visit_binary_expr(ASTBinaryExpr& expr) { walk_binary_expr(expr); }

    virtual void visit_type(ExprType type) { walk_type(type); }
    virtual void visit_name(SourceRange name) { walk_name(name); }

    // These methods shall not be overriden! They would cause the visitation of
    // a node to happen more than once. For instance, the statement `1;` would
    // perform the following sequence of calls `visit_stmt => visit_expr_stmt
    //  => visit_expr => visit_number_expr` all of which processes the same
    // node (an `ASTNumber`) using a different super class.
    //
    // They are still provided as public because they can be helpful to dispatch
    // the visitation of a superclass into a derived class visitation (e.g. call
    // `visit_stmt` in order to reach `visit_iteration_stmt`).
    void visit_decl(ASTDecl& decl) { walk_decl(decl); }
    void visit_stmt(ASTStmt& stmt) { walk_stmt(stmt); }
    void visit_expr_stmt(ASTExpr& stmt) { walk_expr_stmt(stmt); }
    void visit_expr(ASTExpr& expr) { walk_expr(expr); }

public:
    void walk_program(ASTProgram& program);

    void walk_var_decl(ASTVarDecl& var_decl);
    void walk_parm_decl(ASTParmVarDecl& parm_decl);
    void walk_fun_decl(ASTFunDecl& fun_decl);

    void walk_null_stmt(ASTNullStmt& null_stmt);
    void walk_compound_stmt(ASTCompoundStmt& comp_stmt);
    void walk_selection_stmt(ASTSelectionStmt& if_stmt);
    void walk_iteration_stmt(ASTIterationStmt& while_stmt);
    void walk_return_stmt(ASTReturnStmt& retn_stmt);

    void walk_number_expr(ASTNumber& expr);
    void walk_var_expr(ASTVarRef& expr);
    void walk_call_expr(ASTFunCall& expr);
    void walk_binary_expr(ASTBinaryExpr& expr);

    void walk_type(ExprType);
    void walk_name(SourceRange);

private:
    // This is private because their visitor equivalent cannot be overriden.
    void walk_decl(ASTDecl& decl);
    void walk_stmt(ASTStmt& stmt);
    void walk_expr_stmt(ASTStmt& stmt) { visit_expr(*stmt.as_expr()); }
    void walk_expr(ASTExpr& expr);
};
}
