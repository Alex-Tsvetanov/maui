// permissions - Apple (AppKit / macOS) platform partial: the DESKTOP AUTO-GRANT stub, ported
// from Permissions.macos.cs's BasePlatformPermission - CheckStatusAsync / RequestAsync resolve
// Granted for every kind (inline - the C# Task.FromResult), ShouldShowRationale is false, and
// EnsureDeclared validates RequiredInfoPlistKeys, of which the ported kinds declare NONE on
// macOS (location's usage key is only "recommended" there - a console nudge in C#, a no-op
// here). The macOS LocationWhenInUse CoreLocation status override is NOT ported (this unit's
// scope keeps the desktop stub; see permissions.hpp). Compiled as Objective-C++ with ARC for the
// apple backend.

#include <memory>
#include <utility>

#include "maui/essentials/permissions.hpp"

namespace maui::application_model
{
    namespace
    {
        class apple_permission_backend final : public i_permission_backend
        {
        public:
            void check_status(permission_kind /*kind*/, permission_status_callback on_complete) override
            {
                on_complete(permission_status::granted);
            }

            void request(permission_kind /*kind*/, permission_status_callback on_complete) override
            {
                on_complete(permission_status::granted);
            }

            void ensure_declared(permission_kind /*kind*/) override
            {
                // No ported kind has RequiredInfoPlistKeys on macOS.
            }

            [[nodiscard]] bool should_show_rationale(permission_kind /*kind*/) override
            {
                return false;
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_permission_backend> make_permission_backend()
        {
            return std::make_shared<apple_permission_backend>();
        }
    } // namespace detail
} // namespace maui::application_model
