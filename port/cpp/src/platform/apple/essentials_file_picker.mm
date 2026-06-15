// file_picker - Apple (AppKit / native macOS) platform partial. SERVICE SEAM. MAUI ships NO native
// macOS FilePicker partial (only the Android/iOS/Windows/Tizen partials exist; the Mac target is
// MacCatalyst, serviced by FilePicker.ios.cs). The native-macOS analogue is an NSOpenPanel, whose
// runModal needs a running app/run loop the spawned gtest process does not have, so the picking UI is
// not drivable here - the on-host smoke asserts the documented service-seam error, and the predefined
// FilePickerFileType statics mirror the netstandard partial (empty registries -> Value throws). The
// behavioral contract is covered by the headless fake.

#include <memory>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/file_picker.hpp"

namespace maui::storage
{
    namespace
    {
        [[noreturn]] void service_seam_unavailable()
        {
            throw maui::application_model::feature_not_supported(
                "FilePicker requires a presenting app context (NSOpenPanel); it is a service seam not "
                "drivable headlessly.");
        }

        class apple_file_picker final : public i_file_picker
        {
        public:
            void pick_async(const pick_options& /*options*/, file_result_callback /*on_complete*/) override
            {
                service_seam_unavailable();
            }
            void pick_multiple_async(const pick_options& /*options*/, file_results_callback /*on_complete*/) override
            {
                service_seam_unavailable();
            }
        };
    } // namespace

    // No native-macOS Platform*FileType() in MAUI -> the netstandard mirror: empty registries, so
    // value() throws feature_not_supported on the current platform.
    const file_picker_file_type file_picker_file_type::images{};
    const file_picker_file_type file_picker_file_type::png{};
    const file_picker_file_type file_picker_file_type::jpeg{};
    const file_picker_file_type file_picker_file_type::videos{};
    const file_picker_file_type file_picker_file_type::pdf{};

    namespace detail
    {
        std::shared_ptr<i_file_picker> make_file_picker()
        {
            return std::make_shared<apple_file_picker>();
        }
    } // namespace detail
} // namespace maui::storage
