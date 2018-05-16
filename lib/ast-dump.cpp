#include <cminus/ast.hpp>

namespace cminus
{
static auto operation(ASTBinaryExpr::Operation op)
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
    }
}

static void newline(std::string& dest, size_t depth)
{
    if(!dest.empty())
        dest.push_back('\n');
    dest.append(2 * depth, ' ');
}

void ASTProgram::dump(std::string& dest, size_t depth)
{
    newline(dest, depth);
    dest += '[';
    dest += "program";
    dest += ' ';

    for(auto& decl : decls)
        decl->dump(dest, depth + 1);

    newline(dest, depth);
    dest += ']';
}

void ASTVarDecl::dump(std::string& dest, size_t depth)
{
    newline(dest, depth);
    dest += '[';
    dest += "var-declaration";
    dest += ' ';
    dest += "[int]";
    dest += ' ';
    dest += '[';
    dest += this->name;
    dest += ']';
    if(this->is_array())
    {
        assert(this->array_size != nullptr);
        dest += ' ';
        this->array_size->dump(dest, depth + 1);
    }
    dest += ']';
}

void ASTParmVarDecl::dump(std::string& dest, size_t depth)
{
    newline(dest, depth);
    dest += '[';
    dest += "param";
    dest += ' ';
    dest += "[int]";
    dest += ' ';
    dest += '[';
    dest += this->name;
    dest += ']';
    if(this->is_array())
    {
        dest += " [\\[\\]]";
    }
    dest += ']';
}

void ASTFunDecl::dump(std::string& dest, size_t depth)
{
    newline(dest, depth);
    dest += '[';
    dest += "fun-declaration";
    dest += ' ';

    newline(dest, depth + 1);
    dest += (this->is_void_retn ? "[void]" : "[int]");

    newline(dest, depth + 1);
    dest += '[';
    dest += this->name;
    dest += ']';

    newline(dest, depth + 1);
    dest += '[';
    dest += "params";
    for(auto& param : this->params)
    {
        dest += ' ';
        param->dump(dest, depth + 2);
    }
    dest += ']';

    this->comp_stmt->dump(dest, depth + 1);

    newline(dest, depth);
    dest += ']';
}

void ASTNullStmt::dump(std::string& dest, size_t depth)
{
    newline(dest, depth);
    dest += '[';
    dest += ";";
    dest += ']';
}

void ASTCompoundStmt::dump(std::string& dest, size_t depth)
{
    newline(dest, depth);
    dest += '[';
    dest += "compound-stmt";
    dest += ' ';

    for(auto& decl : decls)
        decl->dump(dest, depth + 1);

    for(auto& stmt : stms)
        stmt->dump(dest, depth + 1);

    newline(dest, depth);
    dest += ']';
}

void ASTSelectionStmt::dump(std::string& dest, size_t depth)
{
    newline(dest, depth);
    dest += '[';
    dest += "selection-stmt";
    dest += ' ';

    expr->dump(dest, depth + 1);

    stmt1->dump(dest, depth + 1);

    if(stmt2)
        stmt2->dump(dest, depth + 1);

    newline(dest, depth);
    dest += ']';
}

void ASTIterationStmt::dump(std::string& dest, size_t depth)
{
    newline(dest, depth);
    dest += '[';
    dest += "iteration-stmt";
    dest += ' ';

    expr->dump(dest, depth + 1);

    stmt->dump(dest, depth + 1);

    newline(dest, depth);
    dest += ']';
}

void ASTReturnStmt::dump(std::string& dest, size_t depth)
{
    newline(dest, depth);
    dest += '[';
    dest += "return-stmt";

    if(expr)
    {
        dest += ' ';
        expr->dump(dest, depth + 1);
    }

    dest += ']';
}

void ASTBinaryExpr::dump(std::string& dest, size_t depth)
{
    newline(dest, depth);
    dest += '[';
    dest += operation(this->op);
    dest += ' ';

    left->dump(dest, depth + 1);
    dest += ' ';
    right->dump(dest, depth + 1);

    dest += ']';
}

void ASTNumber::dump(std::string& dest, size_t depth)
{
    dest += '[';
    dest += std::to_string(this->value);
    dest += ']';
}

void ASTVarRef::dump(std::string& dest, size_t depth)
{
    dest += '[';
    dest += "var";
    dest += ' ';
    dest += '[';
    dest += this->decl->get_name();
    dest += ']';
    if(this->expr)
    {
        dest += ' ';
        this->expr->dump(dest, depth + 1);
    }
    dest += ']';
}

void ASTFunCall::dump(std::string& dest, size_t depth)
{
    newline(dest, depth);
    dest += '[';
    dest += "call";

    newline(dest, depth + 1);
    dest += '[';
    dest += this->decl->get_name();
    dest += ']';

    newline(dest, depth + 1);
    dest += '[';
    dest += "args";
    for(auto& expr : args)
    {
        dest += ' ';
        expr->dump(dest, depth + 2);
    }
    dest += ']';

    newline(dest, depth);
    dest += ']';
}
}
