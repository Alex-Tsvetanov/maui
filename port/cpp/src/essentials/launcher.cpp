// The cross-platform half of the launcher facade: the lazily-created implementation slot behind
// Launcher.Default / SetDefault, plus the `new Uri(uri)` format gate of the C# string overloads
// (an RFC 3986 scheme followed by ':'). The implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_launcher.{cpp,mm}) via detail::make_launcher().

#include "maui/essentials/launcher.hpp"

#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace maui::application_model
{
    namespace
    {
        std::shared_ptr<i_launcher>& launcher_storage()
        {
            static std::shared_ptr<i_launcher> storage;
            return storage;
        }
    } // namespace

    i_launcher& launcher::default_()
    {
        auto& storage = launcher_storage();
        if (storage == nullptr)
        {
            storage = detail::make_launcher();
        }
        return *storage;
    }

    void launcher::set_default(std::shared_ptr<i_launcher> implementation)
    {
        launcher_storage() = std::move(implementation);
    }

    namespace detail
    {
        void require_valid_uri(std::string_view uri)
        {
            const std::size_t colon = uri.find(':');
            bool valid = colon != std::string_view::npos && colon > 0 &&
                         std::isalpha(static_cast<unsigned char>(uri.front())) != 0;
            if (valid)
            {
                for (const char character : uri.substr(1, colon - 1))
                {
                    const auto uchar = static_cast<unsigned char>(character);
                    if (std::isalnum(uchar) == 0 && character != '+' && character != '-' && character != '.')
                    {
                        valid = false;
                        break;
                    }
                }
            }
            if (!valid)
            {
                // The UriFormatException of the C# string overloads' `new Uri(uri)`.
                throw std::invalid_argument("Invalid URI: The format of the URI could not be determined.");
            }
        }
    } // namespace detail
} // namespace maui::application_model
