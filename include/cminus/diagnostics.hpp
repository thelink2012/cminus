#pragma once
#include <cminus/sourceman.hpp>
#include <memory>
#include <variant>
#include <vector>
#include <functional>

namespace cminus
{

class DiagnosticManager;
class DiagnosticBuilder;

enum class Diag
{
    lexer_bad_number,
    lexer_bad_char,
};

using DiagArgument = std::variant<int>;

struct Diagnostic
{
    Diag code;
    const SourceFile& source;
    SourceLocation loc;
    std::vector<DiagArgument> args;
    std::vector<SourceRange> ranges;

    explicit Diagnostic(Diag code, const SourceFile& source, SourceLocation loc)
        : code(std::move(code)), source(source), loc(std::move(loc))
    {}
};

class DiagnosticBuilder
{
public:
    explicit DiagnosticBuilder(const SourceFile& source,
                               SourceLocation loc, 
                               Diag code,
                               DiagnosticManager& manager) 
        : manager(manager)
    {
        diag_ptr.reset(new Diagnostic { code, source, loc });
    }

    DiagnosticBuilder(const DiagnosticBuilder&) = delete;
    DiagnosticBuilder(DiagnosticBuilder&&) = default;

    ~DiagnosticBuilder();

    template<typename Arg>
    DiagnosticBuilder& arg(Arg&& arg)
    {
        diag_ptr->args.emplace_back(std::forward<Arg>(arg));
        return *this;
    }

    DiagnosticBuilder& range(SourceRange sr)
    {
        diag_ptr->ranges.emplace_back(std::move(sr));
        return *this;
    }

private:
    std::unique_ptr<Diagnostic> diag_ptr;
    DiagnosticManager& manager;
};

class DiagnosticManager
{
public:
    explicit DiagnosticManager();

    template<typename... Args>
    auto report(const SourceFile& source, SourceLocation loc,
                Diag code, Args&&... args) -> DiagnosticBuilder
    {
        DiagnosticBuilder builder(source, loc, code, *this);
        (builder.arg(std::forward<Args>(args)), ...);
        return builder;
    }

    template<typename F>
    void diag_handler(F handler)
    {
        auto old_diag_handler = std::move(this->curr_diag_handler);
        this->curr_diag_handler = [old_handler = std::move(old_diag_handler),
                                   handler = std::move(handler)]
            (const Diagnostic& diag) {
                if(handler(diag) && old_handler)
                {
                    return old_handler(diag);
                }
                return false;
            };
    }

protected:
    void emit(std::unique_ptr<Diagnostic> diag_ptr);

    friend class DiagnosticBuilder;

private:
    std::function<bool(const Diagnostic&)> curr_diag_handler;
};

}

