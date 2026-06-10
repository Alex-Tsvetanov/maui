#pragma once
// maui::controls::gesture_recognizer  <=  Microsoft.Maui.Controls.GestureRecognizer
// (an Element implementing IGestureRecognizer : INotifyPropertyChanged)
//
// The base class for all gesture recognizers. Deriving the port's element base gives a recognizer the
// same surface the C# original gets from Element: bindable properties (whose changes raise
// bindable_object::property_changed — the INotifyPropertyChanged role of IGestureRecognizer) and a
// logical parent + inherited BindingContext once it is added to a view's gesture_recognizers()
// collection (View's CollectionChanged handler sets item.Parent = this; view<> mirrors that by
// attaching the recognizer as a logical child).
//
// The headers live under include/maui/controls/gestures/ for organization only — the namespace stays
// maui::controls, matching the C# namespace (Microsoft.Maui.Controls has no Gestures sub-namespace).

#include "maui/controls/element.hpp"

namespace maui::controls
{
    class gesture_recognizer : public element
    {
    public:
        gesture_recognizer() = default;
    };
} // namespace maui::controls
