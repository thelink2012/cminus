#include "cminus/sourceman.hpp"
#include <algorithm>
#include <cassert>

namespace cminus
{
SourceFile::SourceFile(std::string source_text_a) :
    source_text(std::move(source_text_a))
{
    // Discover line locations.
    if(!source_text.empty())
    {
        this->lines.push_back(source_text.data());
        for(auto& c : source_text)
        {
            if(c == '\n')
                this->lines.push_back(std::addressof(c) + 1);
        }
    }
}

auto SourceFile::from_stream(std::FILE* stream, size_t hint_size)
        -> std::optional<SourceFile>
{
    std::string source_text;

    // Add one to the hint_size so we can trigger EOF on the first iteration.
    const size_t block_size = (hint_size == -1 ? 4096 : 1 + hint_size);

    while(true)
    {
        auto block_pos = source_text.size();
        source_text.resize(source_text.size() + block_size);

        auto ncount = std::fread(&source_text[block_pos], 1, block_size, stream);

        if(ncount < block_size)
        {
            if(feof(stream))
            {
                source_text.resize(block_pos + ncount);
                break;
            }
            return std::nullopt;
        }
    }

    // Because we added one to the hint_size, we need a threshold of at least one
    // before we shrink_to_fit, otherwise we'll always perform uncessary reallocs.
    if(source_text.capacity() > source_text.size() + 128)
        source_text.shrink_to_fit();

    return SourceFile{std::move(source_text)};
}

auto SourceFile::find_line_and_column(SourceLocation loc) const
        -> std::pair<unsigned, unsigned>
{
    assert(loc >= this->source_text.data()
           && loc <= this->source_text.data() + this->source_text.size() + 1);

    auto it_line_end = std::upper_bound(lines.begin(), lines.end(), loc);
    auto line = static_cast<unsigned>(std::distance(lines.begin(), it_line_end));

    assert(it_line_end != lines.begin());
    auto it_line_begin = std::prev(it_line_end);
    auto column = static_cast<unsigned>(1 + std::distance(*it_line_begin, loc));

    return {line, column};
}

auto SourceFile::view_with_terminator() const -> SourceRange
{
    return SourceRange(source_text.data(), source_text.size() + 1);
}
}
