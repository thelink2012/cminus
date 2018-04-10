#include <cminus/scanner.hpp>
#include <cstdio>
#include <memory>
#include <string>

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
int main(int argc, char* argv[])
{
    // TODO check argc
    
    auto [input_content, input_size] = read_file(argv[1]);

    auto input_view = std::string_view {
        input_content.get(),
        input_size
    };

    Scanner scanner(input_view);
    while(auto word = scanner.next_word())
    {
        std::string x { word->lexeme.begin(), word->lexeme.size() };
        puts(x.c_str());
        if(word->category == Category::Identifier)
            puts("YES!");
    }
}
