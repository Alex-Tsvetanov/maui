// semantic_screen_reader - Apple (AppKit / macOS) platform partial. macOS is NOT SUPPORTED:
// SemanticScreenReader has only a netstandard.tvos.watchos.macos.cs partial (no .macos.cs), whose
// Announce throws NotSupportedOrImplementedException - so the macOS backend throws feature_not_supported
// exactly like that mirror. (VoiceOver-style announcements on macOS would go through
// NSAccessibility.post; MAUI does not ship that, so neither does the port - fidelity over invention.)
// Compiled as Objective-C++ with ARC for the apple backend.

#include <memory>
#include <string_view>

#include "maui/essentials/feature_not_supported.hpp"
#include "maui/essentials/semantic_screen_reader.hpp"

namespace maui::accessibility
{
    namespace
    {
        class apple_semantic_screen_reader final : public i_semantic_screen_reader
        {
        public:
            void announce(std::string_view /*text*/) override
            {
                throw maui::application_model::feature_not_supported();
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_semantic_screen_reader> make_semantic_screen_reader()
        {
            return std::make_shared<apple_semantic_screen_reader>();
        }
    } // namespace detail
} // namespace maui::accessibility
