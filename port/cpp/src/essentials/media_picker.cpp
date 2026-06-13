// The cross-platform half of the media_picker facade: the lazily-created implementation slot behind
// MediaPicker.Default / MediaPicker.SetDefault. The implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_media_picker.{cpp,mm}), reached through
// detail::make_media_picker() - the C# `defaultImplementation ??= new MediaPickerImplementation()`.
// The CapturePhoto/CaptureVideo IsCaptureSupported gate lives in the platform partial (the ios
// partial throws there), so the facade is a thin pass-through.

#include "maui/essentials/media_picker.hpp"

#include <memory>
#include <utility>

namespace maui::media
{
    namespace
    {
        std::shared_ptr<i_media_picker>& media_picker_storage()
        {
            static std::shared_ptr<i_media_picker> storage;
            return storage;
        }
    } // namespace

    i_media_picker& media_picker::default_()
    {
        auto& storage = media_picker_storage();
        if (storage == nullptr)
        {
            storage = detail::make_media_picker();
        }
        return *storage;
    }

    void media_picker::set_default(std::shared_ptr<i_media_picker> implementation)
    {
        media_picker_storage() = std::move(implementation);
    }
} // namespace maui::media
