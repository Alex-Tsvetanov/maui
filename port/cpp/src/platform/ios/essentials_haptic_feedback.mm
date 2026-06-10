// haptic_feedback - iOS (UIKit) platform partial. Ported from HapticFeedback.ios.cs: IsSupported
// is true; Click prepares + fires a Light UIImpactFeedbackGenerator, LongPress a Medium one. (The
// C# 17.5+ view-anchored GetFeedbackGenerator overload is a Catalyst-oriented refinement; the
// plain initWithStyle: path - C#'s fallback branch - is the supported core API and is used here.)

#import <UIKit/UIKit.h>

#include <memory>

#include "maui/essentials/haptic_feedback.hpp"

namespace maui::devices
{
    namespace
    {
        class ios_haptic_feedback final : public i_haptic_feedback
        {
        public:
            [[nodiscard]] bool is_supported() const override
            {
                return true;
            }

            void perform(haptic_feedback_type type) override
            {
                const UIImpactFeedbackStyle style =
                    type == haptic_feedback_type::long_press ? UIImpactFeedbackStyleMedium : UIImpactFeedbackStyleLight;
                UIImpactFeedbackGenerator* const generator = [[UIImpactFeedbackGenerator alloc] initWithStyle:style];
                [generator prepare];
                [generator impactOccurred];
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_haptic_feedback> make_haptic_feedback()
        {
            return std::make_shared<ios_haptic_feedback>();
        }
    } // namespace detail
} // namespace maui::devices
