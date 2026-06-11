// The cross-platform half of the main_thread facade: the lazily-created implementation slot (the
// C# MainThread partial-method seam + the netstandard custom-implementation slot, folded into
// the library's standard current()/set_current pair). The implementation itself is the
// per-backend partial (src/platform/<backend>/essentials_main_thread.{cpp,mm}) via
// detail::make_main_thread().

#include "maui/essentials/main_thread.hpp"

#include <memory>
#include <utility>

namespace maui::application_model
{
    namespace
    {
        std::shared_ptr<i_main_thread>& main_thread_storage()
        {
            static std::shared_ptr<i_main_thread> storage;
            return storage;
        }
    } // namespace

    i_main_thread& main_thread::current()
    {
        auto& storage = main_thread_storage();
        if (storage == nullptr)
        {
            storage = detail::make_main_thread();
        }
        return *storage;
    }

    void main_thread::set_current(std::shared_ptr<i_main_thread> implementation)
    {
        main_thread_storage() = std::move(implementation);
    }
} // namespace maui::application_model
