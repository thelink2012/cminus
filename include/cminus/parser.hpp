#include <cminus/scanner.hpp>

namespace cminus
{
class Parser
{
public:
    explicit Parser(Scanner& scanner) :
        scanner(scanner)
    {
        word_buffer[0] = scanner.next_word();
        word_buffer[1] = scanner.next_word();
    }

    void parse_simple_expression();
    void parse_additive_expression();

private:
    Word current_word() { return word_buffer[0]; }
    void lookahead(size_t n) { /* TODO static assert */ return word_buffer[n]; }

    // DONT CONSUME ON ERROR
    template<typename... Args>
    std::optional<Word> consume(Args&&... args)
    {
        if(word_buffer[0].is_any_of(std::forward<Args>(args)...))
        {
            /* TODO success */
        }
        /* TODO failure */
    }

private:
    Scanner& scanner;
    Word words_buffer[2];
};
}
