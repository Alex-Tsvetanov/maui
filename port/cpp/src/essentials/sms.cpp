// The cross-platform half of the sms facade: the lazily-created implementation slot behind
// Sms.Default / SetDefault. The implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_sms.{cpp,mm}) via detail::make_sms().

#include "maui/essentials/sms.hpp"

#include <memory>
#include <utility>

namespace maui::application_model::communication
{
    namespace
    {
        std::shared_ptr<i_sms>& sms_storage()
        {
            static std::shared_ptr<i_sms> storage;
            return storage;
        }
    } // namespace

    i_sms& sms::default_()
    {
        auto& storage = sms_storage();
        if (storage == nullptr)
        {
            storage = detail::make_sms();
        }
        return *storage;
    }

    void sms::set_default(std::shared_ptr<i_sms> implementation)
    {
        sms_storage() = std::move(implementation);
    }
} // namespace maui::application_model::communication
