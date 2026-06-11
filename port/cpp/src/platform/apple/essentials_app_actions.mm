// app_actions - Apple (AppKit / macOS) platform partial: NOT SUPPORTED. The suffix oracle is
// AppActions.netstandard.tvos.watchos.macos.tizen.cs - macOS is covered only by the netstandard
// partial, so is_supported / get / set ALL throw (C#'s NotImplementedInReferenceAssembly ->
// feature_not_supported). The AppActionActivated event accessors stay subscribable (the C#
// partial declares the event field - adding a handler does not throw); it simply never raises.
// Compiled as Objective-C++ with ARC for the apple backend.

#include <memory>
#include <vector>

#include "maui/essentials/app_actions.hpp"
#include "maui/essentials/feature_not_supported.hpp"

#include "src/essentials/detail/app_actions_base.hpp"

namespace maui::application_model
{
    namespace
    {
        class apple_app_actions final : public detail::app_actions_base
        {
        public:
            [[nodiscard]] bool is_supported() const override
            {
                throw feature_not_supported("This API is not supported on macOS.");
            }

            void get_async(app_actions_callback /*on_complete*/) override
            {
                throw feature_not_supported("This API is not supported on macOS.");
            }

            void set_async(const std::vector<app_action>& /*actions*/) override
            {
                throw feature_not_supported("This API is not supported on macOS.");
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_app_actions> make_app_actions()
        {
            return std::make_shared<apple_app_actions>();
        }
    } // namespace detail
} // namespace maui::application_model
