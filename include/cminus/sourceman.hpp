#pragma once
#include <cstdio>
#include <string_view>
#include <optional>
#include <string>
#include <vector>

namespace cminus
{

class SourceFile;

using SourceRange = std::string_view;

using SourceLocation = const char*;

class SourceFile
{
public:
    explicit SourceFile(std::string);
    SourceFile(const SourceFile&) = delete;
    SourceFile(SourceFile&&) = default;

    static auto from_stream(std::FILE*) -> std::optional<SourceFile>;

    auto view_with_terminator() const -> std::string_view;

    auto find_line_and_column(SourceLocation loc) -> std::pair<unsigned, unsigned>;

private:
    std::string source_data;
    std::vector<const char*> line_offsets;
};


// TODO check if SourceRange and SourceLocation are simple and small (static_assert)

}

