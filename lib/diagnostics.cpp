#include <cassert>
#include <cminus/diagnostics.hpp>

namespace cminus
{
DiagnosticManager::DiagnosticManager()
{
}

void DiagnosticManager::emit(std::unique_ptr<Diagnostic> diag_ptr)
{
    assert(diag_ptr != nullptr);
    curr_diag_handler(*diag_ptr);
}

void DiagnosticManager::handler(std::function<bool(const Diagnostic&)> handler)
{
    auto old_handler = std::move(this->curr_diag_handler);
    this->curr_diag_handler = [old_handler = std::move(old_handler),
                               handler = std::move(handler)](const Diagnostic& diag) {
        if(handler(diag) && old_handler)
        {
            return old_handler(diag);
        }
        return false;
    };
}

DiagnosticBuilder::~DiagnosticBuilder()
{
    this->manager.emit(std::move(diag_ptr));
}
}
