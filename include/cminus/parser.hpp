#include <cminus/diagnostics.hpp>
#include <cminus/scanner.hpp>
#include <cminus/semantics.hpp>

namespace cminus
{
/// The parser tries to contruct a derivation in a grammar from a stream of
/// lexed words. For each derived production, a syntax-directed action gets
/// called to perform further processing, including the construction of an
/// abstract syntax tree node for that derived production.
///
/// This is essentially a bridge between the scanner and the semantic
/// analyzer.
class Parser
{
public:
    explicit Parser(Scanner& scanner,
                    Semantics& sema,
                    DiagnosticManager& diagman) :
        scanner(scanner),
        sema(sema),
        diagman(diagman)
    {
        peek_word = scanner.next_word();
        for(auto& lw : lookahead_words)
            lw = scanner.next_word();
    }

    auto parse_program() -> std::shared_ptr<ASTProgram>;

private:
    auto parse_declaration() -> std::shared_ptr<ASTDecl>;
    auto parse_var_declaration() -> std::shared_ptr<ASTVarDecl>;
    auto parse_fun_declaration() -> std::shared_ptr<ASTFunDecl>;
    auto parse_param() -> std::shared_ptr<ASTParmVarDecl>;

    auto parse_compound_stmt(ScopeFlags) -> std::shared_ptr<ASTCompoundStmt>;
    auto parse_statement() -> std::shared_ptr<ASTStmt>;

    auto parse_expression() -> std::shared_ptr<ASTExpr>;
    auto parse_simple_expression() -> std::shared_ptr<ASTExpr>;
    auto parse_additive_expression() -> std::shared_ptr<ASTExpr>;
    auto parse_term() -> std::shared_ptr<ASTExpr>;
    auto parse_factor() -> std::shared_ptr<ASTExpr>;
    auto parse_number() -> std::shared_ptr<ASTNumber>;
    auto parse_var() -> std::shared_ptr<ASTVarRef>;
    auto parse_call() -> std::shared_ptr<ASTFunCall>;


    /// Looks ahead in the stream by N words.
    ///
    /// Notice `lookahead(0) == peek_word`!
    Word lookahead(size_t n)
    {
        assert(n <= std::size(lookahead_words));
        if(n == 0)
            return peek_word;
        else
            return lookahead_words[n - 1];
    }

    /// \returns the next word in the stream regardless of its category.
    auto consume() -> Word
    {
        auto num_lws = std::size(lookahead_words);
        auto ate_word = std::exchange(peek_word, lookahead_words[0]);
        std::move(&lookahead_words[1], &lookahead_words[num_lws],
                  lookahead_words);
        lookahead_words[num_lws - 1] = scanner.next_word();
        return ate_word;
    }

    /// Tries to consume the next word from the stream.
    ///
    /// \returns the word if the category matches with any in `args...`,
    ///          otherwise `std::nullopt`.
    template<typename... Args>
    auto try_consume(Args&&... args) -> std::optional<Word>
    {
        static_assert(((std::is_same_v<std::decay_t<Args>, Category>)&&...));
        if(peek_word.is_any_of(std::forward<Args>(args)...))
            return consume();
        else
            return std::nullopt;
    }

    /// Tries to consume the next word from the stream. If that is not
    /// possible an error diagnostic is produced.
    ///
    /// \returns the word if the category matches `category`.
    auto expect_and_consume(Category category) -> std::optional<Word>
    {
        if(peek_word.category != category)
        {
            diagman.report(scanner.get_source(), peek_word.location(),
                           Diag::parser_expected_token, category);
            return std::nullopt;
        }
        return consume();
    }

    auto expect_and_consume_type() -> std::optional<Word>;

private:
    Scanner& scanner;
    Semantics& sema;
    DiagnosticManager& diagman;

    /// The next word to be consumed from the stream.
    Word peek_word;
    /// Some more words after the peek word.
    Word lookahead_words[2];
};
}
