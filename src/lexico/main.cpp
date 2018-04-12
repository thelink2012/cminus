#include <cminus/scanner.hpp>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
using namespace cminus;

auto read_file(const char* filename) -> std::pair<std::unique_ptr<const char>, size_t>
{
    FILE* file = fopen(filename, "rb");
    if(file != nullptr)
    {
        fseek(file, 0, SEEK_END);
        long fsize = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *result = new char[fsize + 1];
        fread(result, fsize, 1, file);
        result[fsize] = 0;

        fclose(file);
        return {
            std::unique_ptr<const char> { result },
            size_t(fsize)
        };
    }
    return { nullptr, 0 };
}

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
        case Category::EndOfCode:
            return "EOF";
        default:
            assert(0);
            std::abort();
            // TODO ^ Unreachable macro
    }
}

auto build_line_offsets(std::string_view source) -> std::vector<const char*>
{
    std::vector<const char*> offsets;
    offsets.push_back(source.begin());
    for(auto curr = source.begin(); curr != source.end(); ++curr)
    {
        if(*curr == '\n')
            offsets.push_back(curr + 1);
    }
    return offsets;
}

auto find_line_for_word(const Word& word, const std::vector<const char*>& offsets) -> unsigned
{
    auto it = std::upper_bound(offsets.begin(), offsets.end(), word.lexeme.begin());
    if(it != offsets.end())
        return static_cast<unsigned>(std::distance(offsets.begin(), it));

    assert(0);
    return 0;
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
    
    auto [input_content, input_size] = read_file(argv[1]);

    auto input_view = std::string_view {
        input_content.get(),
        input_size
    };

    Scanner scanner(input_view);
    auto line_offsets = build_line_offsets(input_view);
    while(auto word = scanner.next_word())
    {
        auto catname = category_to_string(word->category);
        auto& lexeme = word->lexeme;
        fprintf(output_stream, "(%u,%.*s,\"%.*s\")\n",
                find_line_for_word(*word, line_offsets),
                static_cast<int>(catname.size()), catname.data(),
                static_cast<int>(lexeme.size()), lexeme.data());
    }

    // TODO use a finally guard instead.
    if(output_stream != stdout)
        fclose(output_stream);
}
