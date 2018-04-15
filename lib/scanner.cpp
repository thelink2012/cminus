#include <cminus/scanner.hpp>
#include <cminus/utility/contracts.hpp>

namespace cminus
{
bool Scanner::is_letter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Scanner::is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

bool Scanner::is_space(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

bool Scanner::lex_identifier(SourceLocation& out_pos)
{
    auto pos = out_pos;
    assert(is_letter(*pos));

    ++pos;
    while(is_letter(*pos) || is_digit(*pos))
        ++pos;

    out_pos = pos;
    return true;
}

bool Scanner::lex_number(SourceLocation& out_pos)
{
    auto pos = out_pos;
    assert(is_digit(*pos));

    ++pos;
    while(is_digit(*pos))
        ++pos;

    if(is_letter(*pos))
        return false;

    out_pos = pos;
    return true;
}

auto Scanner::next_word() -> std::optional<Word>
{
    SourceLocation token_start;

    // We'll use a few labels here. Consider this is an automaton.
    // Alternatives are recursion and a loop. Both cases were
    // discarded as they made things uglier than with labels.
next_word_again:
    token_start = current_pos;
    switch(*current_pos)
    {
        case '\0':
            return std::nullopt;

        case ' ':
        case '\t':
        case '\n':
            ++current_pos;
            while(is_space(*current_pos)) ++current_pos;
            goto next_word_again;

        case '/':
            ++current_pos;
            if(*current_pos == '*')
            {
                // Find the end of the comment and try another word afterwards.
                for(++current_pos; *current_pos; ++current_pos)
                {
                    if(*current_pos == '*' && *std::next(current_pos) == '/')
                    {
                        std::advance(current_pos, 2);
                        goto next_word_again;
                    }
                }

                // End of stream but no end of comment found.
                diagman.report(source, token_start, Diag::lexer_unclosed_comment)
                        .range(SourceRange(token_start, 2));
                return std::nullopt;
            }
            return Word(Category::Divide, token_start, current_pos);

        case '*':
            ++current_pos;
            return Word(Category::Multiply, token_start, current_pos);

        case '-':
            ++current_pos;
            return Word(Category::Minus, token_start, current_pos);

        case '+':
            ++current_pos;
            return Word(Category::Plus, token_start, current_pos);

        case '<':
            ++current_pos;
            if(*current_pos == '=')
            {
                ++current_pos;
                return Word(Category::LessEqual, token_start, current_pos);
            }
            return Word(Category::Less, token_start, current_pos);

        case '>':
            ++current_pos;
            if(*current_pos == '=')
            {
                ++current_pos;
                return Word(Category::GreaterEqual, token_start, current_pos);
            }
            return Word(Category::Greater, token_start, current_pos);

        case '=':
            ++current_pos;
            if(*current_pos == '=')
            {
                ++current_pos;
                return Word(Category::Equal, token_start, current_pos);
            }
            return Word(Category::Assign, token_start, current_pos);

        case '!':
            ++current_pos;
            if(*current_pos == '=')
            {
                ++current_pos;
                return Word(Category::NotEqual, token_start, current_pos);
            }
            --current_pos;
            goto invalid_char;

        case ';':
            ++current_pos;
            return Word(Category::Semicolon, token_start, current_pos);

        case ',':
            ++current_pos;
            return Word(Category::Comma, token_start, current_pos);

        case '(':
            ++current_pos;
            return Word(Category::OpenParen, token_start, current_pos);

        case ')':
            ++current_pos;
            return Word(Category::CloseParen, token_start, current_pos);

        case '[':
            ++current_pos;
            return Word(Category::OpenBracket, token_start, current_pos);

        case ']':
            ++current_pos;
            return Word(Category::CloseBracket, token_start, current_pos);

        case '{':
            ++current_pos;
            return Word(Category::OpenCurly, token_start, current_pos);

        case '}':
            ++current_pos;
            return Word(Category::CloseCurly, token_start, current_pos);

        // clang-format off
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if(lex_number(current_pos))
            {
                return Word(Category::Number, token_start, current_pos);
            }
            else
            {
                // Something is wrong with this number. Skip to the next token.
                while(is_digit(*current_pos) || is_letter(*current_pos))
                    ++current_pos;

                auto bad_lexeme = SourceRange(token_start,
                                              std::distance(token_start, current_pos));
                diagman.report(source, bad_lexeme.begin(), Diag::lexer_bad_number)
                       .range(bad_lexeme);
                goto next_word_again;
            }

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
                SourceRange lexeme(token_start, std::distance(token_start, current_pos));
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

            // this shall never happen because is_identifier cannot fail.
            cminus_unreachable();
            break;
            // clang-format on

        invalid_char:
        default:
        {
            // We found a character that is not part of our alphabet.
            // Give a diagnostic and skip it.
            diagman.report(source, current_pos, Diag::lexer_bad_char)
                    .range(SourceRange(current_pos, 1));
            ++current_pos;
            goto next_word_again;
        }
    }
}
}
