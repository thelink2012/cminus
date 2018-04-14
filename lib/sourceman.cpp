#include "cminus/sourceman.hpp"
#include <cassert>
#include <algorithm>

namespace cminus
{

constexpr size_t BUFFER_SIZE = 4096;

// TODO could add a hint parameter for size of stream
//

SourceFile::SourceFile(std::string source_data_) :
    source_data(std::move(source_data_))
{
    // Build the line offsets data structure.
    if(!source_data.empty())
    {
        this->line_offsets.push_back(source_data.data());
        for(size_t i = 0; i < source_data.size(); ++i)
        {
            if(source_data[i] == '\n')
                this->line_offsets.push_back(&source_data[i]);
        }
    }
}

auto SourceFile::from_stream(std::FILE* stream) -> std::optional<SourceFile>
{
    char buffer[BUFFER_SIZE];
    size_t ncount;

    std::string source_data;

    while((ncount = std::fread(buffer, 1, BUFFER_SIZE, stream)))
    {
        source_data.append(buffer, ncount);

        if(ncount < BUFFER_SIZE)
        {
            if(feof(stream))
                break;
            return std::nullopt;
        }
    }
    
    return SourceFile { std::move(source_data) };
}

auto SourceFile::find_line_and_column(SourceLocation loc) -> std::pair<unsigned, unsigned>
{
    assert(loc >= this->source_data.data()
           && loc <= this->source_data.data() + this->source_data.size());

    auto it = std::upper_bound(line_offsets.begin(), line_offsets.end(), loc);
    auto line = static_cast<unsigned>(std::distance(line_offsets.begin(), it));

    return { line, 1 };
    // TODO find column later on
}

auto SourceFile::view_with_terminator() const -> std::string_view
{
    return std::string_view(source_data.data(), source_data.size() + 1);
}

}

