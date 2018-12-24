#include <cminus/ast-dump-visitor.hpp>

namespace cminus
{
auto ASTDumpVisitor::operation(ASTBinaryExpr::Operation op) -> const char*
{
    switch(op)
    {
        case ASTBinaryExpr::Operation::Plus:
            return "+";
        case ASTBinaryExpr::Operation::Minus:
            return "-";
        case ASTBinaryExpr::Operation::Multiply:
            return "*";
        case ASTBinaryExpr::Operation::Divide:
            return "/";
        case ASTBinaryExpr::Operation::Less:
            return "<";
        case ASTBinaryExpr::Operation::LessEqual:
            return "<=";
        case ASTBinaryExpr::Operation::Greater:
            return ">";
        case ASTBinaryExpr::Operation::GreaterEqual:
            return ">=";
        case ASTBinaryExpr::Operation::Equal:
            return "==";
        case ASTBinaryExpr::Operation::NotEqual:
            return "!=";
        case ASTBinaryExpr::Operation::Assign:
            return "=";
        default:
            assert(false);
    }
}

void ASTDumpVisitor::newline(size_t depth)
{
    if(!dest.empty())
        dest.push_back('\n');
    dest.append(2 * depth, ' ');
}

void ASTDumpVisitor::visit_program(ASTProgram& program)
{
    newline(depth);
    dest += '[';
    dest += "program";

    ++depth;
    walk_program(program);
    --depth;

    newline(depth);
    dest += ']';
}

void ASTDumpVisitor::visit_var_decl(ASTVarDecl& decl)
{
    newline(depth);
    dest += '[';
    dest += "var-declaration";

    ++depth;
    walk_var_decl(decl);
    --depth;

    dest += ']';
}

void ASTDumpVisitor::visit_parm_decl(ASTParmVarDecl& decl)
{
    newline(depth);
    dest += '[';
    dest += "param";

    ++depth;
    walk_parm_decl(decl);
    --depth;

    if(decl.is_array())
        dest += " [\\[\\]]";

    dest += ']';
}

void ASTDumpVisitor::visit_fun_decl(ASTFunDecl& decl)
{
    newline(depth);
    dest += '[';
    dest += "fun-declaration";

    newline(depth + 1);
    dest += (decl.is_void() ? "[void]" : "[int]");

    newline(depth + 1);
    dest += '[';
    dest += decl.get_name();
    dest += ']';

    newline(depth + 1);
    dest += '[';
    dest += "params";
    for(auto it = decl.parm_begin(); it != decl.parm_end(); ++it)
    {
        depth += 2;
        dest += ' ';
        visit_parm_decl(**it);
        depth -= 2;
    }
    dest += ']';

    ++depth;
    visit_compound_stmt(*decl.get_body());
    --depth;

    newline(depth);
    dest += ']';
}

void ASTDumpVisitor::visit_null_stmt(ASTNullStmt&)
{
    newline(depth);
    dest += '[';
    dest += ";";
    dest += ']';
}

void ASTDumpVisitor::visit_compound_stmt(ASTCompoundStmt& comp_stmt)
{
    newline(depth);
    dest += '[';
    dest += "compound-stmt";
    dest += ' ';

    ++depth;
    walk_compound_stmt(comp_stmt);
    --depth;

    newline(depth);
    dest += ']';
}

void ASTDumpVisitor::visit_selection_stmt(ASTSelectionStmt& if_stmt)
{
    newline(depth);
    dest += '[';
    dest += "selection-stmt";
    dest += ' ';

    ++depth;
    walk_selection_stmt(if_stmt);
    --depth;

    newline(depth);
    dest += ']';
}

void ASTDumpVisitor::visit_iteration_stmt(ASTIterationStmt& while_stmt)
{
    newline(depth);
    dest += '[';
    dest += "iteration-stmt";
    dest += ' ';

    ++depth;
    walk_iteration_stmt(while_stmt);
    --depth;

    newline(depth);
    dest += ']';
}

void ASTDumpVisitor::visit_return_stmt(ASTReturnStmt& retn_stmt)
{
    newline(depth);
    dest += '[';
    dest += "return-stmt";

    ++depth;
    walk_return_stmt(retn_stmt);
    --depth;

    dest += ']';
}

void ASTDumpVisitor::visit_binary_expr(ASTBinaryExpr& expr)
{
    newline(depth);
    dest += '[';
    dest += operation(expr.get_operation());
    dest += ' ';

    ++depth;
    walk_binary_expr(expr);
    --depth;

    dest += ']';
}

void ASTDumpVisitor::visit_number_expr(ASTNumber& num)
{
    dest += " [";
    dest += std::to_string(num.get_value());
    dest += ']';
}

void ASTDumpVisitor::visit_var_expr(ASTVarRef& var)
{
    dest += '[';
    dest += "var";

    ++depth;
    walk_var_expr(var);
    --depth;

    dest += ']';
}

void ASTDumpVisitor::visit_call_expr(ASTFunCall& fun_call)
{
    newline(depth);
    dest += '[';
    dest += "call";

    newline(depth + 1);
    dest += '[';
    dest += fun_call.get_decl()->get_name();
    dest += ']';

    newline(depth + 1);
    dest += '[';
    dest += "args";
    for(auto it = fun_call.arg_begin(); it != fun_call.arg_end(); ++it)
    {
        depth += 2;
        dest += ' ';
        visit_expr(**it);
        depth -= 2;
    }
    dest += ']';

    newline(depth);
    dest += ']';
}

void ASTDumpVisitor::visit_type(ExprType type)
{
    switch(type)
    {
        case ExprType::Void:
            dest += " [void]";
            break;
        case ExprType::Int:
            dest += " [int]";
            break;
        case ExprType::Array:
            dest += " [int]";
            break;
    }
}

void ASTDumpVisitor::visit_name(SourceRange name)
{
    dest += " [";
    dest += name;
    dest += ']';
}
}
