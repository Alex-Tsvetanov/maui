#pragma once
// maui::controls::title_bar  <=  Microsoft.Maui.Controls.TitleBar (the basics)
//
// A custom window title bar: Title / Subtitle text plus an optional custom Content view, hosted by the
// window chrome (an NSTitlebarAccessoryViewController on AppKit; a documented no-op on plain iOS — C#
// maps Window.TitleBar on Windows + Mac Catalyst only). Ported from src/Controls/src/Core/TitleBar/
// TitleBar.cs reduced to those basics; the full C# TitleBar is a TemplatedView (Icon, Leading/Trailing
// content, ForegroundColor, visual states, a default ControlTemplate) — out of scope, see STATUS.md.
// An element (not a view<>): it has no handler of its own — the window_handler materializes it from
// the i_title_bar contract, the same collapse as the toolbar chrome.

#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_title_bar.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class title_bar : public element, public maui::core::i_title_bar
    {
    public:
        title_bar()
        {
            this->set_style_target_type<title_bar>();
        }

        // Shared bindable-property descriptors (TitleBar.TitleProperty / SubtitleProperty).
        static const maui::core::bindable_property<std::string>& title_property();
        static const maui::core::bindable_property<std::string>& subtitle_property();

        void set_title(std::string value)
        {
            title_.set(std::move(value));
        }
        void set_subtitle(std::string value)
        {
            subtitle_.set(std::move(value));
        }
        // C# TitleBar.Content — the custom view hosted in the title bar. NON-owning (the caller owns it).
        void set_content(maui::core::i_view* value)
        {
            content_ = value;
        }

        // ---- i_title_bar ----
        [[nodiscard]] std::string_view title() const override
        {
            return title_.get();
        }
        [[nodiscard]] std::string_view subtitle() const override
        {
            return subtitle_.get();
        }
        [[nodiscard]] maui::core::i_view* content() const override
        {
            return content_;
        }

    private:
        maui::core::property<std::string> title_{*this, title_property()};
        maui::core::property<std::string> subtitle_{*this, subtitle_property()};
        maui::core::i_view* content_ = nullptr; // non-owning
    };
} // namespace maui::controls
