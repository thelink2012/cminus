#include "cminus/scanner.hpp"

bool Scanner::is_letter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Scanner::is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

bool Scanner::lex_identifier(const char*& out_pos)
{
    const char* p = out_pos;
    
    if(is_letter(*p))
    {
        for(++p; is_letter(*p) || is_digit(*p); ++p)
        {}
        out_pos = p;
        return true;
    }
    return false;
}

bool Scanner::lex_number(const char*& out_pos)
{
    const char* p = out_pos;

    if(is_digit(*p))
    {
        for(++p; is_digit(*p); ++p)
        {}
        if(is_letter(*p))
        {
            return false;
        }
        out_pos = p;
        return true;
    }
    return false;
}



auto Scanner::next_word() -> std::optional<Word>
{
    const char* token_start = current_pos;
    switch(*current_pos)
    {
        case '\0':
            return Word {
                Category::EndOfCode,
                std::string_view(),
            };

        case '0': case '1': case '2': case '3': case '4': case '5': case '6':
        case '7': case '8': case '9':
            if(lex_number(current_pos))
            {
                return Word {
                    Category::Number,
                    std::string_view(token_start, size_t(current_pos - token_start))
                }; 
            }
            return std::nullopt;

        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': 
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
            if(lex_identifier(current_pos))
            {
                std::string_view lexeme { token_start, size_t(current_pos - token_start) };
                Category category;

                if(lexeme == "if")
                    category = Category::If;
                else if(lexeme == "else")
                    category = Category::Else;
                else if(lexeme == "int")
                    category = Category::Int;
                else if(lexeme == "void")
                    category = Category::Void;
                else if(lexeme == "return")
                    category = Category::Return;
                else if(lexeme == "while")
                    category = Category::While;
                else
                    category = Category::Identifier;

                return Word { category, lexeme };
            }
            return std::nullopt;

        default:
            return std::nullopt;
    }
}

