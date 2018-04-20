#include <cminus/ast.hpp>
#include <cminus/utility/contracts.hpp>

namespace cminus
{
auto ASTBinaryExpr::type_from_category(Category category) -> Operation
{
    switch(category)
    {
        case Category::Plus:
            return Operation::Plus;
        case Category::Minus:
            return Operation::Minus;
        case Category::Multiply:
            return Operation::Multiply;
        case Category::Divide:
            return Operation::Divide;
        case Category::Less:
            return Operation::Less;
        case Category::LessEqual:
            return Operation::LessEqual;
        case Category::Greater:
            return Operation::Greater;
        case Category::GreaterEqual:
            return Operation::GreaterEqual;
        case Category::Equal:
            return Operation::Equal;
        case Category::NotEqual:
            return Operation::NotEqual;
        case Category::Assign:
            return Operation::Assign;
        default:
            cminus_unreachable();
    }
}
}
