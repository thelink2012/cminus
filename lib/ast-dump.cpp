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
    dest.push_back('\n');
    dest.append(2 * depth, ' ');
}

void ASTBinaryExpr::dump(std::string& dest, size_t depth)
{
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

void ASTVar::dump(std::string& dest, size_t depth)
{
    // TODO
}

void ASTCall::dump(std::string& dest, size_t depth)
{
    // TODO
}
}
