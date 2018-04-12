#pragma once
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

    EndOfCode,
};

struct Word
{
    const Category category;
    const std::string_view lexeme;
};

class Scanner
{
public:
    explicit Scanner(std::string_view source) :
        source(std::move(source))
    {
        this->current_pos = this->source.begin();
        assert(*this->source.end() == '\0');
    }

    auto next_word() -> std::optional<Word>;

private:
    std::string_view source;
    const char* current_pos;

    static bool is_letter(char c);
    static bool is_digit(char c);
    static bool is_space(char c);

    bool lex_identifier(const char*& out_pos);
    bool lex_number(const char*& out_pos);
};


}
