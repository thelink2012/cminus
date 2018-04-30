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
            return "assign";
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
    dest += "decvar";
    dest += ' ';
    dest += this->name;
    if(this->array_size)
    {
        dest += ' ';
        this->array_size->dump(dest, depth + 1);
    }
    dest += ']';
}

void ASTFunDecl::dump(std::string& dest, size_t depth)
{
    // STUB!!
    newline(dest, depth);
    dest += '[';
    dest += "decfunc";
    dest += ' ';
    newline(dest, depth + 1);
    dest += "[void]";
    newline(dest, depth + 1);
    dest += "[main]";
    newline(dest, depth + 1);
    dest += "[paramlist]";
    newline(dest, depth + 1);
    dest += "[block ";

    for(auto& t : test)
        t->dump(dest, depth + 2);

    newline(dest, depth + 1);
    dest += ']';

    newline(dest, depth);
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
    dest += this->varname;
    if(this->expr)
    {
        this->expr->dump(dest, depth + 1);
    }
    dest += ']';
}

void ASTFunCall::dump(std::string& dest, size_t depth)
{
    // TODO
}
}
