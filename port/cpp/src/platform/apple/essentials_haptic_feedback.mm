// haptic_feedback - Apple (AppKit / macOS) platform partial. Ported 1:1 from
// HapticFeedback.macos.cs: IsSupported is true, and Perform fires the NSHapticFeedbackManager
// default performer with the Generic pattern - but ONLY for LongPress (Click is a no-op in the C#
// partial; macOS trackpad haptics have no click-feedback convention).

#import <AppKit/AppKit.h>

#include <memory>

#include "maui/essentials/haptic_feedback.hpp"

namespace maui::devices
{
    namespace
    {
        class apple_haptic_feedback final : public i_haptic_feedback
        {
        public:
            [[nodiscard]] bool is_supported() const override
            {
                return true;
            }

            void perform(haptic_feedback_type type) override
            {
                if (type == haptic_feedback_type::long_press)
                {
                    [[NSHapticFeedbackManager defaultPerformer]
                        performFeedbackPattern:NSHapticFeedbackPatternGeneric
                               performanceTime:NSHapticFeedbackPerformanceTimeDefault];
                }
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_haptic_feedback> make_haptic_feedback()
        {
            return std::make_shared<apple_haptic_feedback>();
        }
    } // namespace detail
} // namespace maui::devices
