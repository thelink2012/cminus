#include <cminus/parser.hpp>

namespace cminus
{
void Parser::parse_simple_expression()
{
    parse_additive_expression();
    
    if(auto op_word = consume(Category::LessEqual, Category::Less,
                              Category::Greater, Category::GreaterEqual,
                              Category::Equal, Category::NotEqual))
    {
        parse_additive_expression();
    }
}

void Parser::parse_additive_expression()
{
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
