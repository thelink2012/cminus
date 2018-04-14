#include <cminus/diagnostics.hpp>
#include <cassert>

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

DiagnosticBuilder::~DiagnosticBuilder()
{
    this->manager.emit(std::move(diag_ptr));
}

}
