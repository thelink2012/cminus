#include <cminus/scanner.hpp>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
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
            assert(0);
            std::abort();
            // TODO ^ Unreachable macro
    }
}

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::fprintf(stderr, "usage: ./lexico <source-file> <out-file>\n");
        return 1;
    }

    FILE* output_stream = [&] {
        if(!strcmp(argv[2], "-"))
            return stdout;
        else
            return fopen(argv[2], "wb");
    }();

    SourceFile source = [&]{
        // FIXME no error handling taking place (fopen and unwrap).
        FILE* f = fopen(argv[1], "rb");
        auto resp = SourceFile::from_stream(f).value();
        fclose(f);
        return resp;
    }();

    DiagnosticManager diagman;
    std::optional<std::pair<unsigned, std::string_view>> error;
    diagman.diag_handler([&] (const Diagnostic& diag) {
            auto [line, column] = source.find_line_and_column(diag.loc);
            switch(diag.code)
            {
                case Diag::lexer_bad_number:
                    error = std::pair { line, diag.ranges.front() };
                    break;
                case Diag::lexer_bad_char:
                    error = std::pair { line, diag.ranges.front() };
                    break;
            }
            return true;
    });

    auto print_line = [&](unsigned line, std::string_view catname, std::string_view lexeme) {
        fprintf(output_stream, "(%u,%.*s,\"%.*s\")\n",
                line,
                static_cast<int>(catname.size()), catname.data(),
                static_cast<int>(lexeme.size()), lexeme.data());
    };
    
    Scanner scanner(source, diagman);
    while(auto word = scanner.next_word())
    {
        if(error)
            break;
        auto [line, column] = source.find_line_and_column(word->lexeme.begin());
        auto catname = category_to_string(word->category);
        print_line(line, catname, word->lexeme);
    }

    // Having the error handling body here we avoid duplication of code.
    // That is because an error may happen right before EOF too.
    if(error)
    {
        print_line(error->first, "ERROR", error->second);
    }

    // TODO use a finally guard instead.
    if(output_stream != stdout)
        fclose(output_stream);
}
