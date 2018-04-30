#include <cminus/parser.hpp>
#include <cminus/scanner.hpp>
#include <cminus/semantics.hpp>
#include <cminus/utility/contracts.hpp>
#include <cminus/utility/scope_guard.hpp>
#include <cstring>
using namespace cminus;

int sintatico(std::FILE* istream, std::FILE* ostream)
{
    bool error = false;
    DiagnosticManager diagman;

    auto source = SourceFile::from_stream(istream);
    if(!source)
    {
        std::perror("sintatico: error");
        return 1;
    }

    diagman.handler([&](const Diagnostic&) {
        error = true;
        return true;
    });

    Scanner scanner(*source, diagman);
    Semantics sema(*source, diagman);
    Parser parser(scanner, sema, diagman);

    if(auto ast = parser.parse_program())
    {
        if(!error)
        {
            std::string ast_dump;
            ast->dump(ast_dump, 0);
            std::fprintf(ostream, "%s\n", ast_dump.c_str());
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::fprintf(stderr, "usage: ./sintatico <source-file> <out-file>\n");
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
            std::perror("sintatico: error");
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
            std::perror("sintatico: error");
            return 1;
        }
    }

    return sintatico(istream, ostream);
}
