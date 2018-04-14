#pragma once
#include <utility>

namespace cminus
{
template<typename Callable>
class ScopeGuard
{
public:
    template<typename F>
    explicit ScopeGuard(F&& f) noexcept :
        callback(std::forward<F>(f))
    {
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard& operator=(ScopeGuard&&) = delete;

    ~ScopeGuard()
    {
        if(should_run)
            callback();
    }

    void dismiss() noexcept { should_run = false; }

private:
    Callable callback;
    bool should_run = true;
};

template<typename F>
ScopeGuard(F)->ScopeGuard<F>;
}
