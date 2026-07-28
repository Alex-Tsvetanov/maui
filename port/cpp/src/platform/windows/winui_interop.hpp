#pragma once
// Shared plumbing for the WinUI 3 (C++/WinRT) backend — the Windows twin of src/platform/ios/ios_*_ops.hpp.
//
// MAUI's Windows backend is WinUI 3 (Microsoft.UI.Xaml): ButtonHandler.Windows.cs creates a
// Microsoft.UI.Xaml.Controls.Button, LabelHandler.Windows.cs a TextBlock. So these handlers create the
// SAME native types, which is what makes a pixel comparison against the MAUI reference board meaningful.
//
// C++/WinRT include rule, restated because every file here hits it: include the FULL winrt header for
// every namespace whose MEMBERS you call. The impl/*.0.h headers that arrive transitively only
// forward-declare, so e.g. calling Slider::Value() without winrt/Microsoft.UI.Xaml.Controls.Primitives.h
// fails with "error C3779: a function that returns 'auto' cannot be used before it is defined" — an error
// that does not read as "add an include".

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.h>

#include <string>
#include <string_view>

#include "maui/graphics/color.hpp"

namespace maui::platform::windows
{
    // ---- the void* `native` slot ----------------------------------------------------------------
    // Every *_platform struct stores its native view in a cross-platform `void* native`. A C++/WinRT
    // smart pointer is a class type and cannot live in a void* directly, so it is HEAP-BOXED and the slot
    // owns that box — the exact analogue of the Apple backends' __bridge_retained reference, released by
    // the backend-defined ~*_platform(). take/ref/drop are the three operations; there is no implicit
    // copy, so a slot is freed exactly once.
    // The box is a plain aggregate wrapper, NOT the WinRT type itself: every projected type derives from
    // winrt::Windows::Foundation::IUnknown, which DELETES operator new precisely to stop callers
    // heap-allocating a smart pointer. Wrapping sidesteps that without defeating its intent — the handle
    // is still a value, and the box is what the slot owns.
    template <class T> struct boxed
    {
        T value;
    };

    template <class T> void* take(const T& value)
    {
        return new boxed<T>{value};
    }

    // UB on a null slot by design (matching the Apple twins): every caller already guards on
    // `platform->native == nullptr` before touching the native view, and a silent empty-object fallback
    // would turn "the handler never created its view" into an invisible no-op instead of a crash.
    template <class T> T& ref(void* slot)
    {
        return static_cast<boxed<T>*>(slot)->value;
    }

    template <class T> void drop(void*& slot)
    {
        delete static_cast<boxed<T>*>(slot);
        slot = nullptr;
    }

    // ---- conversions ----------------------------------------------------------------------------
    // The port speaks UTF-8 std::string; WinRT speaks UTF-16 hstring.
    winrt::hstring to_hstring(std::string_view utf8);
    std::string to_utf8(const winrt::hstring& text);

    // maui::graphics::color is normalized float RGBA; Windows::UI::Color is 8-bit BGRA-ordered fields.
    winrt::Windows::UI::Color to_ui_color(const maui::graphics::color& value);
} // namespace maui::platform::windows
