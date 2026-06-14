// The cross-platform half of the clipboard facade: the lazily-created implementation slot behind
// Clipboard.Default / SetDefault. The implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_clipboard.{cpp,mm}) via detail::make_clipboard(); the shared
// add/remove listener gate lives inline in detail::clipboard_base (clipboard.hpp).

#include "maui/essentials/clipboard.hpp"

#include <memory>
#include <utility>

namespace maui::application_model
{
    namespace
    {
        std::shared_ptr<i_clipboard>& clipboard_storage()
        {
            static std::shared_ptr<i_clipboard> storage;
            return storage;
        }
    } // namespace

    i_clipboard& clipboard::default_()
    {
        auto& storage = clipboard_storage();
        if (storage == nullptr)
        {
            storage = detail::make_clipboard();
        }
        return *storage;
    }

    void clipboard::set_default(std::shared_ptr<i_clipboard> implementation)
    {
        clipboard_storage() = std::move(implementation);
    }
} // namespace maui::application_model
