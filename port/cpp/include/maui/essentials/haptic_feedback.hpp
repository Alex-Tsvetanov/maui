#pragma once
// maui::devices::haptic_feedback       <=  Microsoft.Maui.Devices.HapticFeedback (static facade)
// maui::devices::i_haptic_feedback     <=  Microsoft.Maui.Devices.IHapticFeedback
// maui::devices::haptic_feedback_type  <=  Microsoft.Maui.Devices.HapticFeedbackType
//
// Backends (suffix oracle): ios REAL (HapticFeedback.ios.cs - UIImpactFeedbackGenerator: Click ->
// light impact, LongPress -> medium impact). apple/macOS REAL (HapticFeedback.macos.cs -
// NSHapticFeedbackManager; faithfully, only LongPress performs feedback and Click is a no-op).
// Headless mirrors the netstandard partial (everything throws) until faked.

#include <memory>

namespace maui::devices
{
    enum class haptic_feedback_type
    {
        click,
        long_press,
    };

    class i_haptic_feedback
    {
    public:
        virtual ~i_haptic_feedback() = default;

        [[nodiscard]] virtual bool is_supported() const = 0;
        virtual void perform(haptic_feedback_type type) = 0;

    protected:
        i_haptic_feedback() = default;
        i_haptic_feedback(const i_haptic_feedback&) = default;
        i_haptic_feedback(i_haptic_feedback&&) = default;
        i_haptic_feedback& operator=(const i_haptic_feedback&) = default;
        i_haptic_feedback& operator=(i_haptic_feedback&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (HapticFeedbackImplementation), one per backend under
        // src/platform/<backend>/essentials_haptic_feedback.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_haptic_feedback> make_haptic_feedback();
    } // namespace detail

    // The static facade over haptic_feedback::default_() (C# HapticFeedback.Default).
    class haptic_feedback final
    {
    public:
        haptic_feedback() = delete;

        [[nodiscard]] static bool is_supported()
        {
            return default_().is_supported();
        }
        // HapticFeedback.Perform (the C# default argument is Click).
        static void perform(haptic_feedback_type type = haptic_feedback_type::click)
        {
            default_().perform(type);
        }

        // HapticFeedback.Default (lazy platform default) + SetDefault (the test seam; nullptr resets).
        [[nodiscard]] static i_haptic_feedback& default_();
        static void set_default(std::shared_ptr<i_haptic_feedback> implementation);
    };
} // namespace maui::devices
