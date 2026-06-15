#pragma once
// maui::core::i_ios_slider_specifics  <=  (port seam) Microsoft.Maui.Controls.Slider's iOS remap surface
//
// C# wires the iOSSpecific Slider.UpdateOnTap knob by APPENDING a Controls-layer mapping to the Core
// SliderHandler (Slider.Mapper.cs ReplaceMapping("UpdateOnTap", Slider.iOS.cs MapUpdateOnTap →
// SliderHandler.MapUpdateOnTap installing/removing a UITapGestureRecognizer that sets Value from the tap
// location). The port's mapper tables are Core-owned, so the value crosses on an IiOSEntrySpecifics-style
// side contract: controls::slider implements it over the platform-spec store; the per-backend
// map_update_on_tap body dynamic_casts the i_slider to it (the W2-24 platform-configuration pattern).

namespace maui::core
{
    class i_ios_slider_specifics
    {
    public:
        virtual ~i_ios_slider_specifics() = default;

        // C# iOSSpecific.Slider.GetUpdateOnTap (default false): when true, a tap anywhere on the track
        // jumps the value to the tapped position (a UITapGestureRecognizer is attached); false removes it.
        [[nodiscard]] virtual bool update_on_tap() const = 0;

    protected:
        i_ios_slider_specifics() = default;
        i_ios_slider_specifics(const i_ios_slider_specifics&) = default;
        i_ios_slider_specifics(i_ios_slider_specifics&&) = default;
        i_ios_slider_specifics& operator=(const i_ios_slider_specifics&) = default;
        i_ios_slider_specifics& operator=(i_ios_slider_specifics&&) = default;
    };
} // namespace maui::core
