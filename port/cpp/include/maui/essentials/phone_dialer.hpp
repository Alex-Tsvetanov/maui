#pragma once
// maui::application_model::communication::phone_dialer    <=  Microsoft.Maui.ApplicationModel.Communication.PhoneDialer
// (static facade) maui::application_model::communication::i_phone_dialer  <=
// Microsoft.Maui.ApplicationModel.Communication.IPhoneDialer
//
// Opens a phone number in the device dialer. Open(number) is SYNCHRONOUS (void, like the C#), and
// carries the shared ValidateOpen gate, in C#'s exact order: a blank/whitespace number throws
// std::invalid_argument (the ArgumentNullException analog), THEN an unsupported device throws
// feature_not_supported (FeatureNotSupportedException). IsSupported is a synchronous bool.
//
// Backends (suffix oracle): apple/macOS REAL (PhoneDialer.macos.cs - NSWorkspace opens "tel:<number>";
// IsSupported = NSWorkspace can open "tel:0000000000"), ios REAL (PhoneDialer.ios.cs - Launcher opens
// "tel:<number>"; IsSupported = UIApplication can open a tel: URL - false on the simulator, which has
// no phone, so the on-simulator suite asserts the unsupported gate). Headless mirrors netstandard
// (throws until faked: the fake records the dialed number behind a settable is-supported flag).

#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace maui::application_model::communication
{
    class i_phone_dialer
    {
    public:
        virtual ~i_phone_dialer() = default;

        // IsSupported: can this device place calls?
        [[nodiscard]] virtual bool is_supported() const = 0;
        // Open(number): validate (blank -> invalid_argument; unsupported -> feature_not_supported)
        // then open the dialer.
        virtual void open(std::string_view number) = 0;

    protected:
        i_phone_dialer() = default;
        i_phone_dialer(const i_phone_dialer&) = default;
        i_phone_dialer(i_phone_dialer&&) = default;
        i_phone_dialer& operator=(const i_phone_dialer&) = default;
        i_phone_dialer& operator=(i_phone_dialer&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (PhoneDialerImplementation), one per backend under
        // src/platform/<backend>/essentials_phone_dialer.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_phone_dialer> make_phone_dialer();

        // ValidateOpen(number): the shared gate the platform Open() runs first - blank ->
        // std::invalid_argument, then !is_supported -> feature_not_supported. Provided so every
        // backend's open() reproduces the C# order exactly.
        void validate_phone_dialer_open(std::string_view number, bool is_supported);
    } // namespace detail

    // The static facade over phone_dialer::default_() (C# PhoneDialer).
    class phone_dialer final
    {
    public:
        phone_dialer() = delete;

        // IsSupported.
        [[nodiscard]] static bool is_supported()
        {
            return default_().is_supported();
        }
        // Open(number).
        static void open(std::string_view number)
        {
            default_().open(number);
        }

        // PhoneDialer.Default (lazy platform default) + SetDefault (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_phone_dialer& default_();
        static void set_default(std::shared_ptr<i_phone_dialer> implementation);
    };
} // namespace maui::application_model::communication
