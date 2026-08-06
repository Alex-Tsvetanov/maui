// maui::xaml — XAML registration for control group "gestures":
//   TapGestureRecognizer, PanGestureRecognizer, PinchGestureRecognizer, SwipeGestureRecognizer,
//   PointerGestureRecognizer
//
// These are the five recognizers the gallery's gesture pages attach in the CODE-FIRST column
// (examples/gallery/pages/gestures_page.hpp) — the XAML column could not even MINT one before this
// TU, because a recognizer type had no entry in the xaml_type_registry. With them registered, the
// <View.GestureRecognizers> property-element routing in xaml_visitors.cpp
// (try_add_gesture_recognizer) can add each parsed recognizer to the owning view's collection.
//
// Pattern deviation from every other register_xaml_<group>.cpp: NO register_view_properties<T>.
// A GestureRecognizer is an Element, not a View (GestureRecognizer.cs : Element) — it has no
// Margin / HorizontalOptions / Background / Style surface at all, so the shared view attribute
// block neither applies nor compiles here.
//
// NOT registered, deliberately:
//   - DragGestureRecognizer / DropGestureRecognizer: no shared twin in port/maui-reference/pages
//     uses them (drag_drop.xaml explicitly omits the gesture layer), and the code-first gallery does
//     not attach them either. Additive when a page needs them.
//   - CommandParameter (Tap) and the five Pointer*CommandParameter members: the port models C#'s
//     `object` CommandParameter as a boxed std::any that is NOT a property<T> (the typed property
//     engine needs operator== on T — see tap_gesture_recognizer.hpp / pointer_gesture_recognizer.hpp),
//     so there is no bindable_property descriptor to register against. Omitted rather than faked.
//   - PanGestureRecognizer.Command / SwipeGestureRecognizer.Command: the PORT's pan and swipe
//     recognizers carry no Command property at all (a documented port deviation in their headers —
//     they raise the event only), so there is nothing to register. Tap and Pointer do have them.
//   - PinchGestureRecognizer has no bindable property in C# either (IsPinching is read-only state) —
//     the type registration alone is its whole markup surface.
//
// Converters added here (gesture-group-owned, none already in register_standard_xaml_converters):
//   convert_swipe_direction  <=  Microsoft.Maui.SwipeDirection ([Flags]: Right/Left/Up/Down)
//   convert_buttons_mask     <=  Microsoft.Maui.Controls.ButtonsMask ([Flags]: Primary/Secondary)
//   convert_uint32           <=  the `uint` Threshold property's UInt32.Parse
//
// FIDELITY NOTE on SwipeDirection: src/Core/src/Primitives/SwipeDirection.cs declares ONLY
// Right/Left/Up/Down — there is no `None` member, so C# Enum.Parse rejects Direction="None". The
// port's enum spells the 0 default `none` for C++ ergonomics (swipe_direction.hpp says so), but the
// CONVERTER must not accept that name, or markup would parse a value C# rejects. `None` is therefore
// absent from the name table below and Direction="None" is a load error, matching C#.

#include "register_xaml_groups.hpp"

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

#include "maui/controls/gestures/buttons_mask.hpp"
#include "maui/controls/gestures/pan_gesture_recognizer.hpp"
#include "maui/controls/gestures/pinch_gesture_recognizer.hpp"
#include "maui/controls/gestures/pointer_gesture_recognizer.hpp"
#include "maui/controls/gestures/swipe_gesture_recognizer.hpp"
#include "maui/controls/gestures/tap_gesture_recognizer.hpp"
#include "maui/core/swipe_direction.hpp"
#include "maui/xaml/xaml_converter_registry.hpp"
#include "maui/xaml/xaml_converters.hpp"
#include "maui/xaml/xaml_parse_exception.hpp"
#include "maui/xaml/xaml_property_registry.hpp"
#include "maui/xaml/xaml_type_registry.hpp"

namespace maui::xaml
{
    namespace
    {
        // C# Enum.Parse over a [Flags] enum, as TypeConversionExtensions.ConvertTo reaches it: a
        // comma-separated list of member names, each parsed (and trimmed) by the shared parse_enum, all
        // OR-combined. A single name is the common case and behaves exactly like parse_enum.
        // The FIRST token seeds the accumulator rather than a value-initialized E: 0 is not a named
        // enumerator of buttons_mask, so `E result{}` is an out-of-range enum value (the very thing the
        // enums' bit_cast-based operator| exists to avoid).
        template <class E>
        [[nodiscard]] E parse_flags(std::string_view text, std::span<const enum_entry<E>> names, const char* type_name)
        {
            const auto token_at = [text](std::size_t begin, std::size_t end) {
                return text.substr(begin, end == std::string_view::npos ? std::string_view::npos : end - begin);
            };
            std::size_t end = text.find(',');
            E result = parse_enum<E>(token_at(0, end), names, type_name);
            while (end != std::string_view::npos)
            {
                const std::size_t begin = end + 1;
                end = text.find(',', begin);
                result = result | parse_enum<E>(token_at(begin, end), names, type_name);
            }
            return result;
        }

        // Microsoft.Maui.SwipeDirection (SwipeDirection.cs) — see the FIDELITY NOTE above: no `None`.
        [[nodiscard]] maui::core::swipe_direction convert_swipe_direction(std::string_view text)
        {
            using maui::core::swipe_direction;
            static constexpr std::array<enum_entry<swipe_direction>, 4> names{{
                {.name = "Right", .value = swipe_direction::right},
                {.name = "Left", .value = swipe_direction::left},
                {.name = "Up", .value = swipe_direction::up},
                {.name = "Down", .value = swipe_direction::down},
            }};
            return parse_flags<swipe_direction>(text, names, "maui::core::swipe_direction");
        }

        // Microsoft.Maui.Controls.ButtonsMask (ButtonsMask.cs).
        [[nodiscard]] maui::controls::buttons_mask convert_buttons_mask(std::string_view text)
        {
            using maui::controls::buttons_mask;
            static constexpr std::array<enum_entry<buttons_mask>, 2> names{{
                {.name = "Primary", .value = buttons_mask::primary},
                {.name = "Secondary", .value = buttons_mask::secondary},
            }};
            return parse_flags<buttons_mask>(text, names, "maui::controls::buttons_mask");
        }

        // SwipeGestureRecognizer.Threshold is a C# `uint` — UInt32.Parse, so a negative or non-numeric
        // literal is a conversion failure (from_chars rejects the leading '-' for an unsigned type).
        [[nodiscard]] std::uint32_t convert_uint32(std::string_view text)
        {
            const std::string_view value = detail::trim(text);
            std::uint32_t result = 0;
            const char* first = value.data();
            const char* last = std::next(first, static_cast<std::ptrdiff_t>(value.size()));
            const auto [ptr, ec] = std::from_chars(first, last, result);
            if (ec != std::errc{} || ptr != last)
            {
                // C#: throw new InvalidOperationException($"Cannot convert \"{strValue}\" into {type}").
                throw xaml_convert_error(std::format("Cannot convert \"{}\" into std::uint32_t", text));
            }
            return result;
        }

        // The xaml_convert_error -> xaml_parse_exception bridge every group TU carries (mirrors the
        // private registry_converter in xaml_standard_types.cpp / register_xaml_shell.cpp).
        template <class T> [[nodiscard]] auto registry_converter(T (*convert)(std::string_view))
        {
            return [convert](const std::string& text) -> T {
                try
                {
                    return convert(text);
                }
                catch (const xaml_convert_error& error)
                {
                    throw xaml_parse_exception(error.what());
                }
            };
        }
    } // namespace

    void register_xaml_gestures(xaml_type_registry& types, xaml_property_registry& properties,
                                xaml_converter_registry& converters)
    {
        namespace controls = maui::controls;

        converters.register_converter<maui::core::swipe_direction>(registry_converter(&convert_swipe_direction));
        converters.register_converter<controls::buttons_mask>(registry_converter(&convert_buttons_mask));
        converters.register_converter<std::uint32_t>(registry_converter(&convert_uint32));

        // ---- TapGestureRecognizer (TapGestureRecognizer.cs) ----
        // Command is shared_ptr<i_command>: no text converter exists (an ICommand has no literal form) —
        // registering it as a bindable property enables Command="{Binding …}", the GraphicsView.Drawable
        // precedent.
        types.register_type<controls::tap_gesture_recognizer>("TapGestureRecognizer");
        properties.register_bindable_property<controls::tap_gesture_recognizer>(
            "NumberOfTapsRequired", controls::tap_gesture_recognizer::number_of_taps_required_property());
        properties.register_bindable_property<controls::tap_gesture_recognizer>(
            "Buttons", controls::tap_gesture_recognizer::buttons_property());
        properties.register_bindable_property<controls::tap_gesture_recognizer>(
            "Command", controls::tap_gesture_recognizer::command_property());

        // ---- PanGestureRecognizer (PanGestureRecognizer.cs) ----
        types.register_type<controls::pan_gesture_recognizer>("PanGestureRecognizer");
        properties.register_bindable_property<controls::pan_gesture_recognizer>(
            "TouchPoints", controls::pan_gesture_recognizer::touch_points_property());

        // ---- PinchGestureRecognizer (PinchGestureRecognizer.cs; no bindable property) ----
        types.register_type<controls::pinch_gesture_recognizer>("PinchGestureRecognizer");

        // ---- SwipeGestureRecognizer (SwipeGestureRecognizer.cs) ----
        types.register_type<controls::swipe_gesture_recognizer>("SwipeGestureRecognizer");
        properties.register_bindable_property<controls::swipe_gesture_recognizer>(
            "Direction", controls::swipe_gesture_recognizer::direction_property());
        properties.register_bindable_property<controls::swipe_gesture_recognizer>(
            "Threshold", controls::swipe_gesture_recognizer::threshold_property());

        // ---- PointerGestureRecognizer (PointerGestureRecognizer.cs) ----
        types.register_type<controls::pointer_gesture_recognizer>("PointerGestureRecognizer");
        properties.register_bindable_property<controls::pointer_gesture_recognizer>(
            "Buttons", controls::pointer_gesture_recognizer::buttons_property());
        properties.register_bindable_property<controls::pointer_gesture_recognizer>(
            "PointerEnteredCommand", controls::pointer_gesture_recognizer::pointer_entered_command_property());
        properties.register_bindable_property<controls::pointer_gesture_recognizer>(
            "PointerExitedCommand", controls::pointer_gesture_recognizer::pointer_exited_command_property());
        properties.register_bindable_property<controls::pointer_gesture_recognizer>(
            "PointerMovedCommand", controls::pointer_gesture_recognizer::pointer_moved_command_property());
        properties.register_bindable_property<controls::pointer_gesture_recognizer>(
            "PointerPressedCommand", controls::pointer_gesture_recognizer::pointer_pressed_command_property());
        properties.register_bindable_property<controls::pointer_gesture_recognizer>(
            "PointerReleasedCommand", controls::pointer_gesture_recognizer::pointer_released_command_property());
    }
} // namespace maui::xaml
