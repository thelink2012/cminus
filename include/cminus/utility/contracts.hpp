#pragma once
#include <cassert>
#include <stdexcept>

namespace cminus
{
#if !defined(NDEBUG)

// http://stackoverflow.com/a/19343239/2679626
#ifndef STRINGIFY
#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)
#endif

struct unreachable_exception : std::runtime_error
{
    unreachable_exception(const char* msg) :
        std::runtime_error(msg)
    {
    }
};

// Unreachable code must be marked with this.
#define cminus_unreachable() \
    (throw cminus::unreachable_exception("unreachable code reached at " __FILE__ ":" STRINGIFY(__LINE__)))
#else

#if defined(__clang__) || defined(__GNUC__)
#define cminus_unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
#define cminus_unreachable() __assume(0)
#else
#define cminus_unreachable() (std::terminate())
#endif

#endif
}
