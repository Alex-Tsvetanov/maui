// maui::controls — the cross-control text/image seam for drag & drop (drag_drop_data.hpp). The C#
// analogs are ViewExtensions.GetStringValue, Element.TrySetValue, and the SendDrop image assignment;
// this TU is where the concrete-control dispatch lives (so the recognizer headers stay free of the
// controls-layer include fan-out — mirroring C#, where GetStringValue is in ViewExtensions, not the
// recognizer). Coverage + the TimePicker/DatePicker gap are documented in drag_drop_data.hpp.

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/check_box.hpp"
#include "maui/controls/drag_drop_data.hpp"
#include "maui/controls/editor.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/image.hpp"
#include "maui/controls/image_button.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/radio_button.hpp"
#include "maui/controls/toggle_switch.hpp"
#include "maui/core/i_image_source.hpp"

namespace maui::controls
{
    namespace
    {
        // C#'s bool.ToString() — "True" / "False" (the exact strings GetStringValue emits for the
        // toggle controls, and the strings TextPackageCorrectlySetsOnCompatibleTarget feeds back in).
        std::string bool_to_string(bool value)
        {
            return value ? "True" : "False";
        }

        // C#'s bool.TryParse: trims surrounding whitespace, then case-insensitively matches "true" /
        // "false". Returns the parsed value on success, nullopt otherwise (TrySetValue then no-ops).
        std::optional<bool> try_parse_bool(std::string_view text)
        {
            // find_first/last_not_of mirror C#'s leading/trailing whitespace trim.
            const std::size_t first = text.find_first_not_of(" \t\n\v\f\r");
            if (first == std::string_view::npos)
            {
                return std::nullopt; // all whitespace / empty
            }
            const std::size_t last = text.find_last_not_of(" \t\n\v\f\r");
            std::string trimmed(text.substr(first, last - first + 1));
            std::ranges::transform(trimmed, trimmed.begin(),
                                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
            if (trimmed == "true")
            {
                return true;
            }
            if (trimmed == "false")
            {
                return false;
            }
            return std::nullopt;
        }
    } // namespace

    std::optional<std::string> get_string_value(const element& source)
    {
        // ViewExtensions.GetStringValue — the if/else type ladder (TimePicker/DatePicker omitted; see the
        // header note). dynamic_cast is the port's `is`/`as` pattern-match.
        if (const auto* lbl = dynamic_cast<const label*>(&source))
        {
            return std::string(lbl->text());
        }
        if (const auto* ent = dynamic_cast<const entry*>(&source))
        {
            return std::string(ent->text());
        }
        if (const auto* edt = dynamic_cast<const editor*>(&source))
        {
            return std::string(edt->text());
        }
        if (const auto* chk = dynamic_cast<const check_box*>(&source))
        {
            return bool_to_string(chk->is_checked());
        }
        if (const auto* sw = dynamic_cast<const toggle_switch*>(&source))
        {
            return bool_to_string(sw->is_toggled());
        }
        if (const auto* rb = dynamic_cast<const radio_button*>(&source))
        {
            return bool_to_string(rb->is_checked());
        }
        return std::nullopt;
    }

    bool try_set_string_value(element& target, std::string_view text)
    {
        // Element.TrySetValue — note CheckBox is in GetStringValue but NOT TrySetValue (C# parity), and
        // Switch/RadioButton only accept a parseable bool.
        if (auto* lbl = dynamic_cast<label*>(&target))
        {
            lbl->set_text(std::string(text));
            return true;
        }
        if (auto* ent = dynamic_cast<entry*>(&target))
        {
            ent->set_text(std::string(text));
            return true;
        }
        if (auto* edt = dynamic_cast<editor*>(&target))
        {
            edt->set_text(std::string(text));
            return true;
        }
        if (auto* sw = dynamic_cast<toggle_switch*>(&target))
        {
            if (const auto parsed = try_parse_bool(text))
            {
                sw->set_is_toggled(*parsed);
                return true;
            }
        }
        else if (auto* rb = dynamic_cast<radio_button*>(&target))
        {
            if (const auto parsed = try_parse_bool(text))
            {
                rb->set_is_checked(*parsed);
                return true;
            }
        }
        return false;
    }

    bool try_set_image_source(element& target, std::shared_ptr<maui::core::i_image_source> source)
    {
        // DropGestureRecognizer.SendDrop: Parent is Image → image.Source; Parent is ImageButton →
        // ib.Source. (Parent is Button → b.ImageSource is not portable — button has no image source in the
        // port; documented in drag_drop_data.hpp's image-seam scope.)
        if (auto* img = dynamic_cast<image*>(&target))
        {
            img->set_source(std::move(source));
            return true;
        }
        if (auto* ib = dynamic_cast<image_button*>(&target))
        {
            ib->set_source(std::move(source));
            return true;
        }
        return false;
    }
} // namespace maui::controls
