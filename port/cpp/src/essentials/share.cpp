// The cross-platform half of the share facade: the lazily-created implementation slot behind
// Share.Default / SetDefault, plus the request validators of ShareImplementation (Share.shared.cs).
// The implementation itself is the per-backend partial (src/platform/<backend>/essentials_share.
// {cpp,mm}) via detail::make_share().

#include "maui/essentials/share.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

namespace maui::application_model::data_transfer
{
    namespace
    {
        std::shared_ptr<i_share>& share_storage()
        {
            static std::shared_ptr<i_share> storage;
            return storage;
        }
    } // namespace

    i_share& share::default_()
    {
        auto& storage = share_storage();
        if (storage == nullptr)
        {
            storage = detail::make_share();
        }
        return *storage;
    }

    void share::set_default(std::shared_ptr<i_share> implementation)
    {
        share_storage() = std::move(implementation);
    }

    namespace detail
    {
        void validate_share_request(const share_text_request& request)
        {
            // string.IsNullOrEmpty(Text) && string.IsNullOrEmpty(Uri) -> ArgumentException.
            if (request.text.empty() && request.uri.empty())
            {
                throw std::invalid_argument(
                    "Both the Text and Uri are invalid. Make sure to include at least one of them in the request.");
            }
        }

        void validate_share_request(const share_file_request& request)
        {
            // request.File == null -> ArgumentException.
            if (!request.file.has_value())
            {
                throw std::invalid_argument("The File parameter in the request files is invalid");
            }
        }

        void validate_share_request(const share_multiple_files_request& request)
        {
            // !(Files?.Count > 0) -> ArgumentException (an empty list is invalid; there is no null
            // entry to check since share_file is a value type).
            if (request.files.empty())
            {
                throw std::invalid_argument("The Files parameter in the request files is invalid");
            }
        }
    } // namespace detail
} // namespace maui::application_model::data_transfer
