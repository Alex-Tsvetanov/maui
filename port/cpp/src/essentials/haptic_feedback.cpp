// The cross-platform half of the haptic_feedback facade: the lazily-created implementation slot behind
// HapticFeedback.Default / HapticFeedback.SetDefault. The implementation itself is the per-backend
// partial (src/platform/<backend>/essentials_haptic_feedback.{cpp,mm}), reached through
// detail::make_haptic_feedback() - the C# `defaultImplementation ??= new HapticFeedbackImplementation()`.

#include "maui/essentials/haptic_feedback.hpp"

#include <memory>
#include <utility>

namespace maui::devices
{
    namespace
    {
        std::shared_ptr<i_haptic_feedback>& haptic_feedback_storage()
        {
            static std::shared_ptr<i_haptic_feedback> storage;
            return storage;
        }
    } // namespace

    i_haptic_feedback& haptic_feedback::default_()
    {
        auto& storage = haptic_feedback_storage();
        if (storage == nullptr)
        {
            storage = detail::make_haptic_feedback();
        }
        return *storage;
    }

    void haptic_feedback::set_default(std::shared_ptr<i_haptic_feedback> implementation)
    {
        haptic_feedback_storage() = std::move(implementation);
    }
} // namespace maui::devices
