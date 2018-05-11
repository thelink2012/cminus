#include <cminus/scanner.hpp>
#include <cminus/utility/contracts.hpp>
#include <cminus/utility/scope_guard.hpp>
#include <cstring>
using namespace cminus;

auto category_to_string(Category category) -> std::string_view
{
    switch(category)
    {
        case Category::Identifier:
            return "ID";
        case Category::Number:
            return "NUM";
        case Category::Else:
        case Category::If:
        case Category::Int:
        case Category::Return:
        case Category::Void:
        case Category::While:
            return "KEY";
        case Category::Plus:
        case Category::Minus:
        case Category::Multiply:
        case Category::Divide:
        case Category::Less:
        case Category::LessEqual:
        case Category::Greater:
        case Category::GreaterEqual:
        case Category::Equal:
        case Category::NotEqual:
        case Category::Assign:
        case Category::Semicolon:
        case Category::Comma:
        case Category::OpenParen:
        case Category::CloseParen:
        case Category::OpenBracket:
        case Category::CloseBracket:
        case Category::OpenCurly:
        case Category::CloseCurly:
            return "SYM";
        default:
            cminus_unreachable();
    }
}

int lexico(std::FILE* istream, std::FILE* ostream)
{
    std::optional<std::pair<unsigned, SourceRange>> error;
    DiagnosticManager diagman;

    auto source = SourceFile::from_stream(istream);
    if(!source)
    {
        std::perror("lexico: error");
        return 1;
    }

    diagman.handler([&](const Diagnostic& diag) {
        auto [line, column] = source->find_line_and_column(diag.loc);
        if(!diag.ranges.empty())
            error = std::pair{line, diag.ranges.front()};
        else
            error = std::pair{line, SourceRange{0, 0}};
        return true;
    });

    auto print_line = [&](unsigned line,
                          std::string_view catname,
                          std::string_view lexeme) {
        std::fprintf(ostream, "(%u,%.*s,\"%.*s\")\n", line,
                     static_cast<int>(catname.size()), catname.data(),
                     static_cast<int>(lexeme.size()), lexeme.data());
    };

    Scanner scanner(*source, diagman);
    for(auto word = scanner.next_word();
        word.category != Category::Eof;
        word = scanner.next_word())
    {
        if(error)
            break;
        auto [line, column] = source->find_line_and_column(word.lexeme.begin());
        auto catname = category_to_string(word.category);
        print_line(line, catname, word.lexeme);
    }

    if(error)
        print_line(error->first, "ERROR", error->second);

    return 0;
}

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::fprintf(stderr, "usage: ./lexico <source-file> <out-file>\n");
        return 1;
    }

    std::FILE* ostream;
    ScopeGuard ostream_guard([&] { fclose(ostream); });
    if(!strcmp(argv[2], "-"))
    {
        ostream = stdout;
        ostream_guard.dismiss();
    }
    else
    {
        ostream = fopen(argv[2], "wb");
        if(ostream == nullptr)
        {
            ostream_guard.dismiss();
            std::perror("lexico: error");
            return 1;
        }
    }

    std::FILE* istream;
    ScopeGuard istream_guard([&] { fclose(istream); });
    if(!strcmp(argv[1], "-"))
    {
        istream = stdin;
        istream_guard.dismiss();
    }
    else
    {
        istream = fopen(argv[1], "rb");
        if(istream == nullptr)
        {
            istream_guard.dismiss();
            std::perror("lexico: error");
            return 1;
        }
    }

    return lexico(istream, ostream);
}
