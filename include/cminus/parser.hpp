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
        diagman(diagman),
        peek_word(Category::Eof, 0, 0),
        lookahead_word(Category::Eof, 0, 0)
    {
        peek_word = scanner.next_word();
        lookahead_word = scanner.next_word();
    }

    auto parse_expression() -> std::shared_ptr<ASTExpr>;

private:
    auto parse_simple_expression() -> std::shared_ptr<ASTExpr>;
    auto parse_additive_expression() -> std::shared_ptr<ASTExpr>;
    auto parse_term() -> std::shared_ptr<ASTExpr>;
    auto parse_factor() -> std::shared_ptr<ASTExpr>;
    auto parse_number() -> std::shared_ptr<ASTNumber>;
    auto parse_var() -> std::shared_ptr<ASTVar>;
    auto parse_call() -> std::shared_ptr<ASTCall>;

    int32_t number_from_word(const Word&);

    /// Looks ahead in the stream by N words.
    ///
    /// The maximum lookahead of this parser is 2. This
    /// means the value of `n` must be `0 <= n <= 1`.
    ///
    /// Notice `lookahead(0) == peek_word`!
    Word lookahead(size_t n)
    {
        assert(n <= 1);
        if(n == 0)
            return peek_word;
        else
            return lookahead_word;
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
        {
            auto ate_word = std::exchange(peek_word, lookahead_word);
            lookahead_word = scanner.next_word();
            return ate_word;
        }
        return std::nullopt;
    }

private:
    Scanner& scanner;
    DiagnosticManager& diagman;

    /// The next word to be consumed from the stream.
    Word peek_word;
    /// The word after the peek word.
    Word lookahead_word;
};
}
