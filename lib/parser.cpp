#include <cminus/parser.hpp>

// This is a recursive descent parser for the C- language. Three words of
// lookahead are used in order to archieve linear time predictive parsing.
// We could reduce the lookahead, but this is small enough.
//
// The complete grammar for the language can be found at the very bottom of this file.

namespace cminus
{
// <program> ::= <declaration-list>
// <declaration-list> ::= <declaration-list> <declaration> | <declaration>
auto Parser::parse_program() -> std::shared_ptr<ASTProgram>
{
    auto program = sema.act_on_program_start();
    while(peek_word.category != Category::Eof)
    {
        if(auto decl = parse_declaration())
        {
            sema.act_on_decl(program, std::move(decl));
        }
        else
        {
            // TODO how do we recover?
            return nullptr;
        }
    }
    return sema.act_on_program_end(program);
}

// <declaration> ::= <var-declaration> | <fun-declaration>
auto Parser::parse_declaration() -> std::shared_ptr<ASTDecl>
{
    // TODO this is a stub
    return parse_fun_declaration();
}

// <fun-declaration> ::= <type-specifier> ID ( <params> ) <compound-stmt>
auto Parser::parse_fun_declaration() -> std::shared_ptr<ASTFunDecl>
{
    // TODO this is a stub
    try_consume(Category::Void, Category::Int).value();
    try_consume(Category::Identifier).value();
    try_consume(Category::OpenParen).value();
    try_consume(Category::Void).value();
    try_consume(Category::CloseParen).value();
    try_consume(Category::OpenCurly).value();
    std::vector<std::shared_ptr<ASTExpr>> test;
    while(true)
    {
        if(auto expr = parse_expression())
        {
            if(!try_consume(Category::Semicolon))
                return nullptr;
            test.push_back(std::move(expr));
            if(peek_word.category != Category::CloseCurly)
                continue;
            try_consume(Category::CloseCurly).value();
            return std::make_shared<ASTFunDecl>(std::move(test)); // stub ctor
        }
        else
        {
            return nullptr;
        }
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
        if(expr1->is<ASTVarRef>() && try_consume(Category::Assign))
        {
            if(auto expr2 = parse_expression())
            {
                return sema.act_on_assign(expr1, expr2);
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
            if(auto expr2 = parse_additive_expression())
            {
                return sema.act_on_binary_expr(expr1, expr2, op_word->category);
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
    // This production has an simple left recursion. Though we may easily
    // use tail recursive parsing to derive it.
    auto expr1 = parse_term();
    if(expr1)
    {
        while(auto op_word = try_consume(Category::Plus, Category::Minus))
        {
            if(auto expr2 = parse_term())
            {
                expr1 = sema.act_on_binary_expr(expr1, expr2, op_word->category);
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
            if(auto expr2 = parse_factor())
            {
                expr1 = sema.act_on_binary_expr(expr1, expr2, op_word->category);
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
        return sema.act_on_number(*word);
    }
    else
    {
        diagman.report(scanner.get_source(), peek_word.location(),
                       Diag::parser_expected_token, Category::Number);
        return nullptr;
    }
}

// <var> ::= ID | ID [ <expression> ]
auto Parser::parse_var() -> std::shared_ptr<ASTVarRef>
{
    // TODO once we have variable decls.
    return nullptr;
}

// <call> ::= ID ( <args> )
auto Parser::parse_call() -> std::shared_ptr<ASTFunCall>
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
