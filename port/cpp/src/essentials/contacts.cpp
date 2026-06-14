// The cross-platform half of the contacts facade: the lazily-created implementation slot behind
// Contacts.Default / SetDefault. The implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_contacts.{cpp,mm}) via detail::make_contacts().

#include "maui/essentials/contacts.hpp"

#include <memory>
#include <utility>

namespace maui::application_model::communication
{
    namespace
    {
        std::shared_ptr<i_contacts>& contacts_storage()
        {
            static std::shared_ptr<i_contacts> storage;
            return storage;
        }
    } // namespace

    i_contacts& contacts::default_()
    {
        auto& storage = contacts_storage();
        if (storage == nullptr)
        {
            storage = detail::make_contacts();
        }
        return *storage;
    }

    void contacts::set_default(std::shared_ptr<i_contacts> implementation)
    {
        contacts_storage() = std::move(implementation);
    }
} // namespace maui::application_model::communication
