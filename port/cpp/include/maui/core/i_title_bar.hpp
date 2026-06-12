#pragma once
// maui::core::i_title_bar  <=  Microsoft.Maui.ITitleBar (the basics)
//
// The window title-bar contract the window chrome materializes (an NSTitlebarAccessoryViewController on
// AppKit). Ported from src/Core/src/Core/ITitleBar.cs, reduced to the basics this cut maps: Title /
// Subtitle (ITitleBar's text pair) + Content (the hosted custom view, IContentView.Content). C#'s full
// TitleBar is a TemplatedView with Icon / Leading-/TrailingContent / ForegroundColor and a default
// control template — out of scope here (documented in STATUS.md; C# maps TitleBar on Windows +
// Mac Catalyst only).

#include <string_view>

namespace maui::core
{
    class i_view;

    class i_title_bar
    {
    public:
        virtual ~i_title_bar() = default;

        // C# TitleBar.Title.
        [[nodiscard]] virtual std::string_view title() const = 0;
        // C# TitleBar.Subtitle.
        [[nodiscard]] virtual std::string_view subtitle() const = 0;
        // C# TitleBar.Content — the custom view hosted in the title bar, or null. Non-owning.
        [[nodiscard]] virtual i_view* content() const = 0;

    protected:
        i_title_bar() = default;
        i_title_bar(const i_title_bar&) = default;
        i_title_bar(i_title_bar&&) = default;
        i_title_bar& operator=(const i_title_bar&) = default;
        i_title_bar& operator=(i_title_bar&&) = default;
    };
} // namespace maui::core
