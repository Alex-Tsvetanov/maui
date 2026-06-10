#pragma once
// maui::application_model::feature_not_supported  <=  Microsoft.Maui.ApplicationModel.FeatureNotSupportedException
//
// THE one error type of the maui_essentials library (layer 7). Every "this feature does not exist
// on this platform / device" failure across the devices+sensors surface raises this single type;
// the C# originals it folds together (distinguished only by message, per the lib's error-model
// decision) are:
//   - FeatureNotSupportedException            (Microsoft.Maui.ApplicationModel)
//   - NotImplementedInReferenceAssemblyException / ExceptionUtils.NotSupportedOrImplementedException
//     (what every *.netstandard.*.cs partial throws - mirrored by the headless backend's
//     unconfigured fakes)
//   - FeatureNotEnabledException              (geolocation's "Location services are not enabled on
//     device." - a not-enabled distinction the port folds into the message)
// C#'s InvalidOperationException (e.g. a sensor's double-start, geolocation's double-listen) is NOT
// a platform-support failure and maps to std::logic_error instead.
//
// FEASIBILITY MATRIX - derived from the C# filename-suffix oracle (a feature is real on a platform
// iff a non-netstandard partial names that platform; a platform covered only by *.netstandard.*.cs
// is NOT supported there and gets this error):
//
//   feature            | headless (default)     | apple/macOS              | ios (+ simulator notes)
//   -------------------+------------------------+--------------------------+---------------------------------
//   device_info        | throws until faked     | REAL (DeviceInfo.macos)  | REAL (DeviceInfo.ios.tvos.watchos)
//   device_display     | defaults (netstandard  | REAL (unit decision: no  | REAL (DeviceDisplay.ios)
//                      | returns, no throw)     | .macos.cs exists in MAUI;|
//                      |                        | ported per instruction   |
//                      |                        | from the retired         |
//                      |                        | Xamarin.Essentials macOS |
//                      |                        | recipe - see header)     |
//   battery            | throws until faked     | REAL (Battery.macos)     | REAL (Battery.ios.watchos;
//                      |                        |                          | simulator level = -1)
//   flashlight         | throws until faked     | NOT SUPPORTED            | REAL (Flashlight.ios; simulator
//                      |                        | (netstandard...macos)    | has no torch => unsupported)
//   vibration          | throws until faked     | NOT SUPPORTED            | REAL (Vibration.ios)
//   haptic_feedback    | throws until faked     | REAL (HapticFeedback     | REAL (HapticFeedback.ios)
//                      |                        | .macos)                  |
//   accelerometer      | throws until faked     | NOT SUPPORTED            | REAL (CoreMotion; the simulator
//   gyroscope          |   (each sensor's       | (every sensor's macOS is | reports the sensors unavailable
//   magnetometer       |   is_supported mirrors | only in the netstandard  | and delivers NO data - tests
//   compass            |   the netstandard      | partial)                 | assert the lifecycle contract
//   barometer          |   throw)               |                          | only; readings are tested via
//   orientation_sensor |                        |                          | the headless fakes)
//   geolocation        | throws until faked     | REAL (Geolocation.ios.   | REAL (same partial)
//                      |                        | macos - CoreLocation)    |
//   geocoding          | throws until faked     | REAL (Geocoding.ios.tvos.| REAL (same partial; geocoding
//                      |                        | watchos.macos-CLGeocoder)| needs network - the headless
//                      |                        |                          | fake is the test path)

#include <stdexcept>
#include <string>

namespace maui::application_model
{
    class feature_not_supported : public std::runtime_error
    {
    public:
        // The default message mirrors FeatureNotSupportedException's intent; the netstandard mirror
        // and the not-enabled fold pass their own messages.
        feature_not_supported() : std::runtime_error("This feature is not supported on the current platform.")
        {
        }
        explicit feature_not_supported(const std::string& message) : std::runtime_error(message)
        {
        }
    };
} // namespace maui::application_model
