#include <cminus/parser.hpp>

// This is a recursive descent parser for the C- language. Two words of
// lookahead are used in order to archieve linear time predictive parsing.
//
// The complete grammar for the language can be found at the very bottom of this file.

namespace cminus
{
int32_t Parser::number_from_word(const Word& word)
{
    assert(word.category == Category::Number);
    try
    {
        // TODO use std::from_chars once it's available in libstdc++
        std::string lexeme(word.lexeme.begin(), word.lexeme.size());
        return std::stoi(lexeme, nullptr, 10);
    }
    catch(const std::out_of_range&)
    {
        diagman.report(scanner.get_source(), word.location(),
                       Diag::parser_number_too_big)
                .range(word.lexeme);
        return 0;
    }
}

// <expression> ::= <var> = <expression> | <simple-expression>
auto Parser::parse_expression() -> std::shared_ptr<ASTExpr>
{
    if(auto expr1 = parse_simple_expression())
    {
        // We can predict whether we should derive an assignment operation
        // or a <simple-expression> by checking whether it returned a <var>.
        //
        // This works because <simple-expression> may only derive an binary expression
        // or an unitary expression (i.e. atomics or in parens). The assignment is
        // more or less an binary expression, but it cannot be derived from the
        // <simple-expression> production. Thus, once it finds a <var> followed
        // by an '=' token in the lookahead, it stops and derives the <var>.
        //
        // Our job is, then, to eat the '=' token and derive the assignment into <var>.
        if(expr1->is<ASTVar>() && try_consume(Category::Assign))
        {
            if(auto expr2 = parse_expression())
            {
                return std::make_shared<ASTAssignExpr>(expr1, expr2);
            }
            else
            {
                return nullptr;
            }
        }
        return expr1;
    }
    return nullptr;
}

// <simple-expression> ::= <additive-expression> <relop> <additive-expression>
//                       | <additive-expression>
// <relop> ::= <= | < | > | >= | == | !=
auto Parser::parse_simple_expression() -> std::shared_ptr<ASTExpr>
{
    if(auto expr1 = parse_additive_expression())
    {
        // This is an easy one to predict since there is no left-recursion.
        if(auto op_word = try_consume(Category::LessEqual, Category::Less,
                                      Category::Greater, Category::GreaterEqual,
                                      Category::Equal, Category::NotEqual))
        {
            auto type = ASTBinaryExpr::type_from_category(op_word->category);
            if(auto expr2 = parse_additive_expression())
            {
                return std::make_shared<ASTBinaryExpr>(expr1, expr2, type);
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return expr1;
        }
    }
    return nullptr;
}

// <additive-expression> ::= <additive-expression> <addop> <term> | <term>
// <addop> ::= + | -
auto Parser::parse_additive_expression() -> std::shared_ptr<ASTExpr>
{
    // This production has an simple left recursion. Though We may easily
    // use tail recursive parsing to derive it.
    auto expr1 = parse_term();
    if(expr1)
    {
        while(auto op_word = try_consume(Category::Plus, Category::Minus))
        {
            auto type = ASTBinaryExpr::type_from_category(op_word->category);
            if(auto expr2 = parse_term())
            {
                auto new_node = std::make_shared<ASTBinaryExpr>(expr1, expr2, type);
                expr1 = std::move(new_node);
            }
            else
            {
                return nullptr;
            }
        }
        return expr1;
    }
    return nullptr;
}

// <term> ::= <term> <mulop> <factor> | <factor>
auto Parser::parse_term() -> std::shared_ptr<ASTExpr>
{
    // This is a soft copy of <additive-expression>. Check that out for details.
    auto expr1 = parse_factor();
    if(expr1)
    {
        while(auto op_word = try_consume(Category::Multiply, Category::Divide))
        {
            auto type = ASTBinaryExpr::type_from_category(op_word->category);
            if(auto expr2 = parse_factor())
            {
                auto new_node = std::make_shared<ASTBinaryExpr>(expr1, expr2, type);
                expr1 = std::move(new_node);
            }
            else
            {
                return nullptr;
            }
        }
        return expr1;
    }
    return nullptr;
}

// <factor> ::= ( <expression> ) | <var> | <call> | NUM
auto Parser::parse_factor() -> std::shared_ptr<ASTExpr>
{
    switch(peek_word.category)
    {
        // NUM
        case Category::Number:
            return parse_number();

        // ( <expression> )
        case Category::OpenParen:
        {
            try_consume(Category::OpenParen).value();

            auto expr1 = parse_expression();
            if(!expr1)
                return nullptr;

            if(!try_consume(Category::CloseParen))
            {
                diagman.report(scanner.get_source(), peek_word.location(),
                               Diag::parser_expected_token, Category::CloseParen);
                return nullptr;
            }

            return expr1;
        }

        // <var> | <call>
        case Category::Identifier:
        {
            // This is easy to predict because nor <var> nor <call> locations
            // are complicated lvalue expressions. Their first two words
            // are always atomic tokens.
            //
            // Hence we introduce an additional lookahead word to check whether
            // this is a function call or not.
            if(lookahead(1).category == Category::OpenParen)
                return parse_call();
            else
                return parse_var();
        }

        default:
        {
            diagman.report(scanner.get_source(), peek_word.location(),
                           Diag::parser_expected_expression);
            return nullptr;
        }
    }
}

// NUM
auto Parser::parse_number() -> std::shared_ptr<ASTNumber>
{
    if(auto word = try_consume(Category::Number))
    {
        auto number = this->number_from_word(*word);
        return std::make_shared<ASTNumber>(number);
    }
    else
    {
        diagman.report(scanner.get_source(), peek_word.location(),
                       Diag::parser_expected_token, Category::Number);
        return nullptr;
    }
}

// <var> ::= ID | ID [ <expression> ]
auto Parser::parse_var() -> std::shared_ptr<ASTVar>
{
    // TODO once we have variable decls.
    return nullptr;
}

// <call> ::= ID ( <args> )
auto Parser::parse_call() -> std::shared_ptr<ASTCall>
{
    // TODO once we have function decls
    return nullptr;
}
}

/*
The following is a list of "challenges" for parsing the grammar provided by
the assignment. The solutions are simple under recursive descendent parsing,
but it is good to be aware of them (specially for building tests).

declaration-list is left-recursive

param-list is left-recursive

local-declarations is left-recursive
local-declaration requires a follow set for backtrack-free parsing

statement-list is left-recursive
statement-list requires a follow set for backtrack-free parsing

additive-expression is left-recursive

term is left-recursive

arg-list is left-recursive

selection-stmt has the dangling else ambiguity
selection-stmt is not backtrack-free, in fact it is not even LL(1)

declaration is not backtrack-free

var-declaration is not backtrack-free

params is not backtrack-free

param is not backtrack-free

args requires a follow set for backtrack-free parsing

FIRST(statement) = FIRST(expression-stmt) U {'{'} U {'if'} U {'while'} U {'return'}
FIRST(expression-stmt) = FIRST(expression) U {';'}
all good, but

FIRST(expression) = FIRST(var) U FIRST(simple-expression)
BUT FIRST(var) and FIRST(simple-expression) are not disjoint
IN OTHER WORDS expression is not backtrack-free

FIRST(simple-expression) = FIRST(var) U FIRST(call) U {'(', NUM}
BUT FIRST(var) and FIRST(call) are not disjoint
IN OTHER WORDS simple-expression is not backtrack-free
THE ROOT of the problem is in rule <factor>
*/

/*
<program> ::= <declaration-list>
<declaration-list> ::= <declaration-list> <declaration> | <declaration>
<declaration> ::= <var-declaration> | <fun-declaration>

<var-declaration> ::= <type-specifier> ID ; | <type-specifier> ID [ NUM ] ;
<type-specifier> ::= int | void

<fun-declaration> ::= <type-specifier> ID ( <params> ) <compound-stmt>
<params> ::= <param-list> | void
<param-list> ::= <param-list> , <param> | <param>
<param> ::= <type-specifier> ID | <type-specifier> ID [ ] 

<compound-stmt> ::= { <local-declarations> <statement-list> }

<local-declarations> ::= <local-declarations> <var-declaration> | empty
<statement-list> ::= <statement-list> <statement> | empty

<statement> ::= <expression-stmt> | <compound-stmt> | <selection-stmt> 
              | <iteration-stmt> | <return-stmt>
<expression-stmt> ::= <expression> ; | ;

<selection-stmt> ::= if ( <expression> ) <statement> 
                  | if ( <expression> ) <statement> else <statement>

<iteration-stmt> ::= while ( <expression> ) <statement>

<return-stmt> ::= return ; | return <expression> ;

<expression> ::= <var> = <expression> | <simple-expression>
<var> ::= ID | ID [ <expression> ]

<simple-expression> ::= <additive-expression> <relop> <additive-expression> 
                      | <additive-expression>
<relop> ::= <= | < | > | >= | == | !=

<additive-expression> ::= <additive-expression> <addop> <term> | <term>
<addop ::= + | -
<term> ::= <term> <mulop> <factor> | <factor>
<mulop> ::= * | /

<factor> ::= ( <expression> ) | <var> | <call> | NUM

<call> ::= ID ( <args> )
<args> ::= <arg-list> | empty
<arg-list> ::= <arg-list> , <expression> | <expression>
*/
