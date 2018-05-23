#pragma once
#include <cminus/sourceman.hpp>
#include <functional>
#include <memory>
#include <variant>
#include <vector>

namespace cminus
{
class DiagnosticManager;
class DiagnosticBuilder;

enum class Category;

/// Diagnostic enumeration.
enum class Diag
{
    lexer_bad_number,
    lexer_bad_char,
    lexer_unclosed_comment,

    parser_expected_token, // %0 => Category
    parser_expected_type,
    parser_expected_expression,
    parser_expected_statement,
    parser_number_too_big,

    sema_redefinition,          // %0 => SymbolName
    sema_undeclared_identifier, // %0 => SymbolName
    sema_empty_program,
    sema_last_decl_not_main,
    sema_var_cannot_be_void,
    sema_assignment_type_error,
    sema_binary_expr_type_error,
    sema_array_statement,
    sema_expr_not_boolean,
    sema_void_fun_returning_value,
    sema_incompatible_return_type,
    sema_int_fun_not_returning_value,
    sema_var_is_not_var,
    sema_index_is_not_int,
    sema_fun_is_not_fun,
    sema_arg_too_few_params,
    sema_arg_too_many_params,
    sema_arg_type_mismatch,
};

/// Parameter for `Diag` printing.
using DiagParam = std::variant<Category, SourceRange>;

/// Diagnostic information.
struct Diagnostic
{
    const SourceFile& source;
    SourceLocation loc;

    Diag code;
    std::vector<DiagParam> args;
    std::vector<SourceRange> ranges;

    explicit Diagnostic(const SourceFile& source, SourceLocation loc, Diag code) :
        source(source), loc(loc), code(code)
    {
    }
};

/// Helper for chain-building a `Diagnostic` and propagating it.
///
/// Use a `DiagnosticManager` to create a instance of this builder
/// through a `.report` method call.
class DiagnosticBuilder
{
public:
    explicit DiagnosticBuilder(const SourceFile& source,
                               SourceLocation loc,
                               Diag code,
                               DiagnosticManager& manager) :
        manager(manager)
    {
        diag_ptr.reset(new Diagnostic{source, loc, code});
    }

    DiagnosticBuilder(const DiagnosticBuilder&) = delete;
    DiagnosticBuilder& operator=(const DiagnosticBuilder&) = delete;

    DiagnosticBuilder(DiagnosticBuilder&&) = default;
    DiagnosticBuilder& operator=(DiagnosticBuilder&&) = default;

    ~DiagnosticBuilder();

    /// Appends an argument for replacement during message formatting.
    template<typename Arg>
    DiagnosticBuilder& arg(Arg&& arg)
    {
        diag_ptr->args.emplace_back(std::forward<Arg>(arg));
        return *this;
    }

    /// Appends a range to be highlighted on the diagnostic.
    DiagnosticBuilder& range(SourceRange sr)
    {
        diag_ptr->ranges.emplace_back(std::move(sr));
        return *this;
    }

private:
    DiagnosticManager& manager;
    std::unique_ptr<Diagnostic> diag_ptr;
};

class DiagnosticManager
{
public:
    explicit DiagnosticManager();

    DiagnosticManager(const DiagnosticManager&) = delete;
    DiagnosticManager& operator=(const DiagnosticManager&) = delete;

    /// Reports a compiler diagnostic.
    template<typename... Args>
    auto report(const SourceFile& source, SourceLocation loc,
                Diag code, Args&&... args) -> DiagnosticBuilder
    {
        DiagnosticBuilder builder(source, loc, code, *this);
        (builder.arg(std::forward<Args>(args)), ...);
        return builder;
    }

    /// Reports a compiler diagnostic without location information.
    template<typename... Args>
    auto report(const SourceFile& source, Diag code, Args&&... args)
            -> DiagnosticBuilder
    {
        // TODO remove the need for a source location.
        auto loc = source.view_with_terminator().begin();
        return report(source, loc, code, std::forward<Args>(args)...);
    }

    /// Replaces the diagnostic handler with another handler.
    ///
    /// The diagnostic handler receives the diagnostic as soon as it is
    /// emitted and its task is to [presumably] print it to the user.
    ///
    /// The handler must return a boolean. If true, the next handler
    /// in the chain will be called as well (i.e. the handler that was in
    /// use before this replacement call took place).
    void handler(std::function<bool(const Diagnostic&)> handler);

protected:
    /// Propagates the diagnostic to a handler.
    void emit(std::unique_ptr<Diagnostic> diag_ptr);

    friend class DiagnosticBuilder;

private:
    std::function<bool(const Diagnostic&)> curr_diag_handler;
};

// The builder must be a small object.
static_assert(sizeof(DiagnosticBuilder) <= 2 * sizeof(size_t));
}
