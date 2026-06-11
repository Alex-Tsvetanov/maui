// The cross-platform half of the browser facade: the lazily-created implementation slot behind
// Browser.Default / SetDefault. The implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_browser.{cpp,mm}) via detail::make_browser().

#include "maui/essentials/browser.hpp"

#include <memory>
#include <utility>

namespace maui::application_model
{
    namespace
    {
        std::shared_ptr<i_browser>& browser_storage()
        {
            static std::shared_ptr<i_browser> storage;
            return storage;
        }
    } // namespace

    i_browser& browser::default_()
    {
        auto& storage = browser_storage();
        if (storage == nullptr)
        {
            storage = detail::make_browser();
        }
        return *storage;
    }

    void browser::set_default(std::shared_ptr<i_browser> implementation)
    {
        browser_storage() = std::move(implementation);
    }
} // namespace maui::application_model
