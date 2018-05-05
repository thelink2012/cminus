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
    do
    {
        if(auto decl = parse_declaration())
            sema.act_on_top_level_decl(program, std::move(decl));
        else
            return nullptr; // TODO how can we recover?
    } while(peek_word.category != Category::Eof);
    return sema.act_on_program_end(program);
}

// <declaration> ::= <var-declaration> | <fun-declaration>
auto Parser::parse_declaration() -> std::shared_ptr<ASTDecl>
{
    // The common prefix of a var-declaration and a fun-declaration
    // is the type-specifier (always atomic) and the identifier (also atomic).
    // Thus we can just lookahaed three words to check whether this is an
    // open paren, meaning a function declaration.
    if(lookahead(2).category == Category::OpenParen)
        return parse_fun_declaration();
    else
        return parse_var_declaration();
}

// <var-declaration> ::= <type-specifier> ID ; | <type-specifier> ID [ NUM ] ;
auto Parser::parse_var_declaration() -> std::shared_ptr<ASTVarDecl>
{
    auto type = expect_and_consume_type();
    if(!type)
        return nullptr;

    auto id = expect_and_consume(Category::Identifier);
    if(!id)
        return nullptr;

    std::shared_ptr<ASTNumber> num;
    if(peek_word.category == Category::OpenBracket)
    {
        consume();

        num = parse_number();
        if(!num)
            return nullptr;

        if(!expect_and_consume(Category::CloseBracket))
            return nullptr;
    }

    if(!expect_and_consume(Category::Semicolon))
        return nullptr;

    return sema.act_on_var_decl(*type, *id, std::move(num));
}

// <type-specifier> ::= int | void
auto Parser::expect_and_consume_type() -> std::optional<Word>
{
    if(auto type = try_consume(Category::Void, Category::Int))
    {
        return type;
    }
    else
    {
        diagman.report(scanner.get_source(), peek_word.location(),
                       Diag::parser_expected_type);
        return std::nullopt;
    }
}

// <fun-declaration> ::= <type-specifier> ID ( <params> ) <compound-stmt>
// <params> ::= <param-list> | void
// <param-list> ::= <param-list> , <param> | <param>
auto Parser::parse_fun_declaration() -> std::shared_ptr<ASTFunDecl>
{
    auto retn = expect_and_consume_type();
    if(!retn)
        return nullptr;

    auto id = expect_and_consume(Category::Identifier);
    if(!id)
        return nullptr;

    if(!expect_and_consume(Category::OpenParen))
        return nullptr;

    std::shared_ptr<ASTCompoundStmt> comp_stmt;
    std::vector<std::shared_ptr<ASTParmVarDecl>> params;
    {
        // Enter a new scope context for the parameters.
        // Keep it active while parsing the function body as well.
        ParseScope scope(sema, ScopeFlags::FunParamsScope);

        // <params> ::= <param-list> | void
        if(lookahead(0).category == Category::Void && lookahead(1).category == Category::CloseParen)
        {
            // The params of the function is a single void, i.e. no params.
            // Consume the `void` and go on.
            consume();
        }
        else // <param-list> ::= <param-list> , <param> | <param>
        {
            if(auto param = parse_param())
                params.push_back(param);
            else
                return nullptr;

            while(peek_word.category != Category::CloseParen)
            {
                if(!expect_and_consume(Category::Comma))
                    return nullptr;

                if(auto param = parse_param())
                    params.push_back(param);
                else
                    return nullptr;
            }
        }

        if(!expect_and_consume(Category::CloseParen))
            return nullptr;

        comp_stmt = parse_compound_stmt(ScopeFlags::CompoundStmt
                                        | ScopeFlags::FunScope);
        if(!comp_stmt)
            return nullptr;
    }

    return sema.act_on_fun_decl(*retn, *id, std::move(params), std::move(comp_stmt));
}

// <param> ::= <type-specifier> ID | <type-specifier> ID [ ]
auto Parser::parse_param() -> std::shared_ptr<ASTParmVarDecl>
{
    auto type = expect_and_consume_type();
    if(!type)
        return nullptr;

    auto id = expect_and_consume(Category::Identifier);
    if(!id)
        return nullptr;

    bool is_array = false;
    if(try_consume(Category::OpenBracket))
    {
        is_array = true;
        if(!expect_and_consume(Category::CloseBracket))
            return nullptr;
    }

    return sema.act_on_param_decl(*type, *id, is_array);
}

// <statement> ::= <expression-stmt> | <compound-stmt> | <selection-stmt>
//              | <iteration-stmt> | <return-stmt>
auto Parser::parse_statement() -> std::shared_ptr<ASTStmt>
{
    // Here we can just look at the category of the current word to decide wich parse is needed.
    switch(peek_word.category){
        case Category::Identifier:
        case Category::Number:
        case Category::OpenParen:
        case Category::Semicolon:
            return parse_expr_stmt();
        case Category::OpenCurly:
            return parse_compound_stmt(ScopeFlags::CompoundStmt);
        case Category::If:
            return parse_selection_stmt();
        case Category::While:
            return parse_iteration_stmt();
        case Category::Return:
            return parse_return_stmt();
        default:
            return nullptr;
    }

}

// <expression-stmt> ::= <expression> ; | ;
auto Parser::parse_expr_stmt() -> std::shared_ptr<ASTStmt>
{
    if(try_consume(Category::Semicolon))
        return sema.act_on_null_stmt();

    if(auto expr = parse_expression())
    {
        if(!expect_and_consume(Category::Semicolon))
            return nullptr;
        return expr;
    }
    return nullptr;
}

// <compound-stmt> ::= { <local-declarations> <statement-list> }
// <local-declarations> ::= <local-declarations> <var-declaration> | empty
// <statement-list> ::= <statement-list> <statement> | empty
auto Parser::parse_compound_stmt(ScopeFlags scope_flags)
        -> std::shared_ptr<ASTCompoundStmt>
{
    if(!expect_and_consume(Category::OpenCurly))
        return nullptr;

    // Enter a new scope context for this compound statement.
    ParseScope scope(sema, scope_flags);
    std::vector<std::shared_ptr<ASTVarDecl>> decls;
    std::vector<std::shared_ptr<ASTStmt>> stms;

    // The first and follow set for local-declaration are disjoint. Therefore
    // we can parse local-declaration as long as we have a valid first symbol.
    // That is, no need to check the follow set when the first symbol is invalid.
    while(peek_word.category == Category::Void
          || peek_word.category == Category::Int)
    {
        if(auto decl = parse_var_declaration())
        {
            decls.push_back(std::move(decl));
        }
        else
        {
            // TODO oh hey there is probably a nice way to recover by
            // skipping until after the next semicolon.
            return nullptr;
        }
    }

    // The first set for statement-list does not contain a '}', but that is
    // the only element from its follow set. That means we can parse as long
    // as we don't find a closing curly bracket.
    while(peek_word.category != Category::CloseCurly)
    {
        if(auto stmt = parse_statement())
        {
            stms.push_back(std::move(stmt));
        }
        else
        {
            // TODO we can recover nicely here as well.
            return nullptr;
        }
    }

    // The only way we can "stop" parsing a compound statement is by
    // reaching in the closing curly.
    assert(peek_word.category == Category::CloseCurly);
    consume();

    return sema.act_on_compound_stmt(std::move(decls), std::move(stms));
}

// <selection-stmt> ::= if ( <expression> ) <statement>
//                  | if ( <expression> ) <statement> else <statement>
auto Parser::parse_selection_stmt() -> std::shared_ptr<ASTSelectionStmt>
{
    if(!expect_and_consume(Category::If))
        return nullptr;
    if(!expect_and_consume(Category::OpenParen))
        return nullptr;

    // Once consumed "if (" we always need to consume some expression.
    if(auto expr = parse_expression())
    {
        if(!expect_and_consume(Category::CloseParen))
            return nullptr;

        // Then we always need to cosnume a statement
        if(auto stmt1 = parse_statement())
        {
            // Once consumed the if's statement is need to check if there's an "else"
            // and if there's our job is parse it.
            if(!try_consume(Category::Else))
                return sema.act_on_selection_stmt(std::move(expr), std::move(stmt1));
            
            if(auto stmt2 = parse_statement())
                return sema.act_on_selection_stmt(std::move(expr), std::move(stmt1), std::move(stmt2));
            return nullptr;
        }
        return nullptr;
    }
    return nullptr;
}

// <iteration-stmt> ::= while ( <expression> ) <statement>
auto Parser::parse_iteration_stmt() -> std::shared_ptr<ASTIterationStmt>
{
    if(!expect_and_consume(Category::While))
        return nullptr;
    if(!expect_and_consume(Category::OpenParen))
        return nullptr;

    if(auto expr = parse_expression())
    {
        if(!expect_and_consume(Category::CloseParen))
            return nullptr;

        if(auto stmt = parse_statement())
            return sema.act_on_iteration_stmt(std::move(expr), std::move(stmt));
        return nullptr;
    }
    return nullptr;
}

// <return-stmt> ::= return ; | return <expression> ;
auto Parser::parse_return_stmt() -> std::shared_ptr<ASTReturnStmt>
{
    if(!expect_and_consume(Category::Return))
        return nullptr;

    if(try_consume(Category::Semicolon))
        return sema.act_on_return_stmt();

    if(auto expr = parse_expression())
    {
        if(!expect_and_consume(Category::Semicolon))
            return nullptr;
        return sema.act_on_return_stmt(std::move(expr));
    }
    return nullptr;
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
                auto lvalue = std::static_pointer_cast<ASTVarRef>(expr1);
                return sema.act_on_assign(std::move(lvalue), expr2);
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
                return sema.act_on_binary_expr(expr1, expr2, op_word->category);
            else
                return nullptr;
        }
        return expr1;
    }
    return nullptr;
}

// <additive-expression> ::= <additive-expression> <addop> <term> | <term>
// <addop> ::= + | -
auto Parser::parse_additive_expression() -> std::shared_ptr<ASTExpr>
{
    // This production has an simple left recursion. Though we may easily
    // use tail recursive parsing to derive it.
    if(auto expr1 = parse_term())
    {
        while(auto op_word = try_consume(Category::Plus, Category::Minus))
        {
            if(auto expr2 = parse_term())
                expr1 = sema.act_on_binary_expr(expr1, expr2, op_word->category);
            else
                return nullptr;
        }
        return expr1;
    }
    return nullptr;
}

// <term> ::= <term> <mulop> <factor> | <factor>
auto Parser::parse_term() -> std::shared_ptr<ASTExpr>
{
    // This is a soft copy of <additive-expression>. Check that out for details.
    if(auto expr1 = parse_factor())
    {
        while(auto op_word = try_consume(Category::Multiply, Category::Divide))
        {
            if(auto expr2 = parse_factor())
                expr1 = sema.act_on_binary_expr(expr1, expr2, op_word->category);
            else
                return nullptr;
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
            consume();

            auto expr1 = parse_expression();
            if(!expr1)
                return nullptr;

            if(!expect_and_consume(Category::CloseParen))
                return nullptr;

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
    if(auto word = expect_and_consume(Category::Number))
        return sema.act_on_number(*word);
    else
        return nullptr;
}

// <var> ::= ID | ID [ <expression> ]
auto Parser::parse_var() -> std::shared_ptr<ASTVarRef>
{
    auto id = expect_and_consume(Category::Identifier);
    if(!id)
        return nullptr;

    std::shared_ptr<ASTExpr> index;
    if(peek_word.category == Category::OpenBracket)
    {
        consume();

        index = parse_expression();
        if(!index)
            return nullptr;

        if(!expect_and_consume(Category::CloseBracket))
            return nullptr;
    }

    return sema.act_on_var(*id, std::move(index));
}

// <call> ::= ID ( <args> )
auto Parser::parse_call() -> std::shared_ptr<ASTFunCall>
{
    // TODO
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

params is almost ambigous.

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
