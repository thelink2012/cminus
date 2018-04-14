#pragma once
#include <cminus/sourceman.hpp>
#include <cminus/diagnostics.hpp>
#include <cassert>
#include <string_view>
#include <optional>

namespace cminus
{

enum class Category
{
    Identifier,
    Number,

    Else,
    If,
    Int,
    Return,
    Void,
    While,

    Plus,
    Minus,
    Multiply,
    Divide,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal,
    NotEqual,
    Assign,
    Semicolon,
    Comma,
    OpenParen,
    CloseParen,
    OpenBracket,
    CloseBracket,
    OpenCurly,
    CloseCurly,
};

struct Word
{
    const Category category;
    const SourceRange lexeme;

    explicit Word(Category category, SourceRange lexeme)
        : category(category), lexeme(lexeme)
    {}

    explicit Word(Category category, const char* begin, const char* end)
        : category(category), lexeme(begin, std::distance(begin, end))
    {}
};

class Scanner
{
public:
    explicit Scanner(const SourceFile& source,
                     DiagnosticManager& diagman) :
        source(source), diagman(diagman)
    {
        auto source_view = source.view_with_terminator();
        assert(*std::prev(source_view.end()) == '\0');
        this->current_pos = source_view.begin();
    }

    auto next_word() -> std::optional<Word>;

private:
    const SourceFile& source;
    DiagnosticManager& diagman;
    const char* current_pos;

    static bool is_letter(char c);
    static bool is_digit(char c);
    static bool is_space(char c);

    bool lex_identifier(const char*& out_pos);
    bool lex_number(const char*& out_pos);

};


}
