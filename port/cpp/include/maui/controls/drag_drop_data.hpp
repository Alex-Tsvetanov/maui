#pragma once
// The cross-control text/image seam the drag & drop recognizers use to auto-extract a drag payload from
// a source element and auto-inject a dropped payload onto a target element. The C# analogs are
// ViewExtensions.GetStringValue (src/Controls/src/Core/ViewExtensions.cs), Element.TrySetValue
// (src/Controls/src/Core/Element/Element.cs), and the IImageElement.Source / Image.Source assignment in
// DropGestureRecognizer.SendDrop. They live OUTSIDE the recognizer (a free-function file that depends on
// the concrete controls) exactly as C# keeps GetStringValue in ViewExtensions, not on the recognizer —
// the recognizer headers stay free of the controls-layer include fan-out, the dispatch lives in
// drag_drop_data.cpp.
//
// Coverage / deviation (documented): the string seam dispatches over the cleanly-portable controls —
// label / entry / editor (string text) and check_box / switch / radio_button (bool, "True"/"False",
// matching C#'s bool.ToString() / bool.TryParse). TimePicker / DatePicker are the gap: C#'s
// GetStringValue / TrySetValue round-trip them through DateTime/TimeSpan default ToString() / TryParse(),
// which are .NET CULTURE-specific facilities the port does not reproduce (CLAUDE.md: don't fabricate a
// .NET-specific behavior). So a drag from / drop onto a time/date picker carries no auto text (the
// explicit Data.Text a handler sets is unaffected). The image seam injects an image source onto
// image / image_button targets (Image.Source / ImageButton.Source); the drag-side image AUTO-extraction
// (Data.Image ??= sourceElement.Source) is not ported — the image controls expose only a borrowed
// i_image_source*, not the shared_ptr ownership the package needs (a user-set Data.Image is unaffected).

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "maui/core/i_image_source.hpp"

namespace maui::controls
{
    class element;

    // ViewExtensions.GetStringValue — the text representation of a compatible source element, or nullopt
    // for an incompatible / unported one (time/date pickers; see the file note). label/entry/editor →
    // their text; check_box/switch/radio_button → "True"/"False".
    [[nodiscard]] std::optional<std::string> get_string_value(const element& source);

    // Element.TrySetValue — push `text` onto a compatible target element. Returns true iff the element
    // type accepted it. label/entry/editor take the raw string; switch/radio_button parse it as a bool
    // ("True"/"true"/… → set, otherwise no-op returning false), matching C#'s bool.TryParse guard.
    bool try_set_string_value(element& target, std::string_view text);

    // The DropGestureRecognizer.SendDrop image-target assignment: set `source` onto an image-bearing
    // target (Image / ImageButton). Returns true iff the element type accepted it.
    bool try_set_image_source(element& target, std::shared_ptr<maui::core::i_image_source> source);
} // namespace maui::controls
