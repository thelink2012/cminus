#pragma once
#include <cminus/ast-visitor.hpp>

namespace cminus
{
///
/// .section data
/// label: word; 
//
/// .section text
/// label:
///
/// acumulador: $v0
///
/// OPTemp: $t0
///
/// esp = $sp
/// ebp = $fp
/// 
/// push é:
///  sw value, 0($sp)
///  sub $sp, $sp, 4
///
/// pop é:
///  lw value, 0($sp)
///  add $sp, $sp, 4
///
/// chamada de funcao:
/// push argN...
/// push arg1
/// jal label
/// add esp, N*4
///
/// dentro da funcao:
/// 
///   inicio da funcao:
///   push ra
///   push ebp
///   ebp = esp+8 // i.e. [ebp] = arg1
///
///   alocar espaco em esp
///   sub esp, 4*num_locals // i.e. [ebp-12] é a primeira local
///
///   no retorno de funcao
///   $v0 = valor de retorno
///   pop ebp
///   pop ra
///   jr ra
///
///   parametros:
///   [ebp+(N*4)] ; N comeca de 0
/// 
/// 
/// 
///
///
///
class ASTCodegenVisitor : public ASTVisitor
{
public:
    explicit ASTCodegenVisitor(std::string& dest) :
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
    std::string& dest;
};
}
