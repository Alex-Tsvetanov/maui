// The cross-platform half of the phone_dialer facade: the lazily-created implementation slot behind
// PhoneDialer.Default / SetDefault, plus the shared ValidateOpen gate (PhoneDialer.shared.cs). The
// implementation itself is the per-backend partial (src/platform/<backend>/essentials_phone_dialer.
// {cpp,mm}) via detail::make_phone_dialer().

#include "maui/essentials/phone_dialer.hpp"

#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "maui/essentials/feature_not_supported.hpp"

namespace maui::application_model::communication
{
    namespace
    {
        std::shared_ptr<i_phone_dialer>& phone_dialer_storage()
        {
            static std::shared_ptr<i_phone_dialer> storage;
            return storage;
        }

        // string.IsNullOrWhiteSpace: empty or all-whitespace.
        [[nodiscard]] bool is_blank(std::string_view value)
        {
            return value.find_first_not_of(" \t\n\v\f\r") == std::string_view::npos;
        }
    } // namespace

    i_phone_dialer& phone_dialer::default_()
    {
        auto& storage = phone_dialer_storage();
        if (storage == nullptr)
        {
            storage = detail::make_phone_dialer();
        }
        return *storage;
    }

    void phone_dialer::set_default(std::shared_ptr<i_phone_dialer> implementation)
    {
        phone_dialer_storage() = std::move(implementation);
    }

    namespace detail
    {
        void validate_phone_dialer_open(std::string_view number, bool is_supported)
        {
            // C# ValidateOpen order: blank number first (ArgumentNullException), then the support
            // gate (FeatureNotSupportedException).
            if (is_blank(number))
            {
                throw std::invalid_argument("number");
            }
            if (!is_supported)
            {
                throw feature_not_supported();
            }
        }
    } // namespace detail
} // namespace maui::application_model::communication
