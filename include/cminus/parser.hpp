#include <cminus/ast.hpp>
#include <cminus/diagnostics.hpp>
#include <cminus/scanner.hpp>

namespace cminus
{
/// The parser consumes a stream of words and spits the abstract
/// syntax tree of the program.
class Parser
{
public:
    explicit Parser(Scanner& scanner,
                    DiagnosticManager& diagman) :
        scanner(scanner),
        diagman(diagman)
    {
        peek_word = scanner.next_word();
        for(auto& lw : lookahead_words)
            lw = scanner.next_word();
    }

    auto parse_program() -> std::shared_ptr<ASTProgram>;

private:
    auto parse_expression() -> std::shared_ptr<ASTExpr>;
    auto parse_simple_expression() -> std::shared_ptr<ASTExpr>;
    auto parse_additive_expression() -> std::shared_ptr<ASTExpr>;
    auto parse_term() -> std::shared_ptr<ASTExpr>;
    auto parse_factor() -> std::shared_ptr<ASTExpr>;
    auto parse_number() -> std::shared_ptr<ASTNumber>;
    auto parse_var() -> std::shared_ptr<ASTVar>;
    auto parse_call() -> std::shared_ptr<ASTCall>;

    // the following are stubs for testing
    auto parse_declaration() -> std::shared_ptr<ASTDecl>;
    auto parse_fun_declaration() -> std::shared_ptr<ASTFunDecl>;

    int32_t number_from_word(const Word&);
    bool is_fundamental_type(const Word&) const;

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
    /// \returns The word if the category matches with any in `args...`,
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

private:
    Scanner& scanner;
    DiagnosticManager& diagman;

    /// The next word to be consumed from the stream.
    Word peek_word;
    /// Some more words after the peek word.
    Word lookahead_words[2];
};
}
