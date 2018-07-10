#include <cminus/ast-codegen-visitor.hpp>
#include <cminus/parser.hpp>
#include <cminus/scanner.hpp>
#include <cminus/semantics.hpp>
#include <cminus/utility/contracts.hpp>
#include <cminus/utility/scope_guard.hpp>
#include <cstring>
using namespace cminus;

std::string_view crt_code = R"__mips__(
.text
.globl __crt_out_of_bounds
.globl println
.globl input

__crt_out_of_bounds:
li $v0, 10 # exit
syscall

println:
li $v0, 1  # print_int
syscall
li $a0, 0x0a
li $v0, 11 # print_char
syscall
j $ra

input:
li $v0, 5 # read_int
syscall
j $ra
)__mips__";

int codegen(std::FILE* istream, std::FILE* ostream)
{
    bool error = false;
    DiagnosticManager diagman;

    auto source = SourceFile::from_stream(istream);
    if(!source)
    {
        std::perror("geracodigo: error");
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
            std::string codegen;
            ASTCodegenVisitor visitor(codegen);
            visitor.visit_program(*ast);
            std::fprintf(ostream, "%s\n", codegen.c_str());
            std::fprintf(ostream, "%*s\n", (int) crt_code.size(), crt_code.data());
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        std::fprintf(stderr, "usage: ./geracodigo <source-file> <out-file>\n");
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
            std::perror("geracodigo: error");
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
            std::perror("geracodigo: error");
            return 1;
        }
    }

    return codegen(istream, ostream);
}
