// The cross-platform half of the file_picker facade: the lazily-created implementation slot behind
// FilePicker.Default / FilePicker.SetDefault. The implementation itself is the per-backend partial
// (src/platform/<backend>/essentials_file_picker.{cpp,mm}), reached through detail::make_file_picker()
// - the C# `defaultImplementation ??= new FilePickerImplementation()`. The PickAsync ->
// PlatformPickAsync(...).FirstOrDefault() narrowing and the PickMultipleAsync pass-through live in the
// platform partial / fake, so the facade is a thin pass-through.

#include "maui/essentials/file_picker.hpp"

#include <memory>
#include <utility>

namespace maui::storage
{
    namespace
    {
        std::shared_ptr<i_file_picker>& file_picker_storage()
        {
            static std::shared_ptr<i_file_picker> storage;
            return storage;
        }
    } // namespace

    i_file_picker& file_picker::default_()
    {
        auto& storage = file_picker_storage();
        if (storage == nullptr)
        {
            storage = detail::make_file_picker();
        }
        return *storage;
    }

    void file_picker::set_default(std::shared_ptr<i_file_picker> implementation)
    {
        file_picker_storage() = std::move(implementation);
    }
} // namespace maui::storage
