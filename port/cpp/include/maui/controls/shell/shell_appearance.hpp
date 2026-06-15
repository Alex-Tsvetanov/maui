#pragma once
// maui::controls::shell_appearance  <=  Microsoft.Maui.Controls.ShellAppearance
// maui::controls::i_shell_appearance_element  <=  Microsoft.Maui.Controls.IShellAppearanceElement
//
// The RESOLVED appearance for a shell pivot: the effective chrome colors (nav bar / tab bar / flyout)
// computed by walking the shell hierarchy from the current page up to the root, ingesting the Shell.*
// color ATTACHED properties at each level (the first level to set a value wins, lowest-in-the-tree
// first). Ported from ShellAppearance.cs + IShellAppearanceElement.cs.
//
// VALUE MODEL (mirrors the C# arrays): ten chrome colors, each "set" or "unset" (the C# nullable Color,
// null = unset), plus FlyoutWidth / FlyoutHeight (the C# double[], -1 = unset). Unset is std::nullopt
// here — the C++ analog of the C# null/-1 sentinels — exactly matching the navigation_page bar-styling
// "set vs unset" precedent (a color the developer never set leaves the chrome on its system default).
//
// DEVIATION (documented, not stubbed): Shell.FlyoutBackdrop is a Microsoft.Maui.Controls.Brush, and the
// controls Brush type is NOT ported yet. So FlyoutBackdrop is omitted from this value type and its ingest
// (the C# s_ingestBrushArray slot). Every Color and the two doubles ARE ingested with full fidelity; the
// backdrop is the single deferred slot (recorded in port/STATUS.md). When Brush lands, add a backdrop
// slot to the ingest the same way the colors are done here.
//
// Equality (ShellAppearance.Equals / operator==): two appearances are equal when every color slot and
// both doubles are equal (the backdrop slot is not modeled). Used by the chrome to skip a no-op re-apply.

#include <array>
#include <cstddef>
#include <optional>

#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class element;

    // The five EFFECTIVE tab-bar colors (IShellAppearanceElement): each tab-bar color falls back to its
    // shell-wide counterpart when unset (TabBarBackgroundColor ?? BackgroundColor, etc.). A pure-shape
    // contract — but the chrome consumes it polymorphically off the resolved appearance, so it is an
    // abstract class (PROFILE §11: runtime polymorphism ⇒ i_*). Effective colors are nullopt only when
    // BOTH the tab-bar slot and its fallback are unset.
    class i_shell_appearance_element
    {
    public:
        i_shell_appearance_element() = default;
        virtual ~i_shell_appearance_element() = default;
        i_shell_appearance_element(const i_shell_appearance_element&) = default;
        i_shell_appearance_element(i_shell_appearance_element&&) = default;
        i_shell_appearance_element& operator=(const i_shell_appearance_element&) = default;
        i_shell_appearance_element& operator=(i_shell_appearance_element&&) = default;

        [[nodiscard]] virtual std::optional<maui::graphics::color> effective_tab_bar_background_color() const = 0;
        [[nodiscard]] virtual std::optional<maui::graphics::color> effective_tab_bar_disabled_color() const = 0;
        [[nodiscard]] virtual std::optional<maui::graphics::color> effective_tab_bar_foreground_color() const = 0;
        [[nodiscard]] virtual std::optional<maui::graphics::color> effective_tab_bar_title_color() const = 0;
        [[nodiscard]] virtual std::optional<maui::graphics::color> effective_tab_bar_unselected_color() const = 0;
    };

    class shell_appearance final : public i_shell_appearance_element
    {
    public:
        shell_appearance() = default;

        // ---- the ten chrome colors (ShellAppearance.<Color>; nullopt = unset) ----
        [[nodiscard]] std::optional<maui::graphics::color> background_color() const
        {
            return colors_[index_background];
        }
        [[nodiscard]] std::optional<maui::graphics::color> disabled_color() const
        {
            return colors_[index_disabled];
        }
        [[nodiscard]] std::optional<maui::graphics::color> foreground_color() const
        {
            return colors_[index_foreground];
        }
        [[nodiscard]] std::optional<maui::graphics::color> tab_bar_background_color() const
        {
            return colors_[index_tab_bar_background];
        }
        [[nodiscard]] std::optional<maui::graphics::color> tab_bar_disabled_color() const
        {
            return colors_[index_tab_bar_disabled];
        }
        [[nodiscard]] std::optional<maui::graphics::color> tab_bar_foreground_color() const
        {
            return colors_[index_tab_bar_foreground];
        }
        [[nodiscard]] std::optional<maui::graphics::color> tab_bar_title_color() const
        {
            return colors_[index_tab_bar_title];
        }
        [[nodiscard]] std::optional<maui::graphics::color> tab_bar_unselected_color() const
        {
            return colors_[index_tab_bar_unselected];
        }
        [[nodiscard]] std::optional<maui::graphics::color> title_color() const
        {
            return colors_[index_title];
        }
        [[nodiscard]] std::optional<maui::graphics::color> unselected_color() const
        {
            return colors_[index_unselected];
        }

        // ---- the flyout dimensions (ShellAppearance.FlyoutWidth / FlyoutHeight; nullopt = unset) ----
        [[nodiscard]] std::optional<double> flyout_width() const
        {
            return flyout_width_;
        }
        [[nodiscard]] std::optional<double> flyout_height() const
        {
            return flyout_height_;
        }

        // ---- IShellAppearanceElement (effective tab-bar colors) ----
        [[nodiscard]] std::optional<maui::graphics::color> effective_tab_bar_background_color() const override
        {
            return tab_bar_background_color() ? tab_bar_background_color() : background_color();
        }
        [[nodiscard]] std::optional<maui::graphics::color> effective_tab_bar_disabled_color() const override
        {
            return tab_bar_disabled_color() ? tab_bar_disabled_color() : disabled_color();
        }
        [[nodiscard]] std::optional<maui::graphics::color> effective_tab_bar_foreground_color() const override
        {
            return tab_bar_foreground_color() ? tab_bar_foreground_color() : foreground_color();
        }
        [[nodiscard]] std::optional<maui::graphics::color> effective_tab_bar_title_color() const override
        {
            return tab_bar_title_color() ? tab_bar_title_color() : title_color();
        }
        [[nodiscard]] std::optional<maui::graphics::color> effective_tab_bar_unselected_color() const override
        {
            return tab_bar_unselected_color() ? tab_bar_unselected_color() : unselected_color();
        }

        // ShellAppearance.Ingest(Element pivot): pull each Shell.* color / flyout-dimension attached value
        // SET on `pivot` into this appearance — but only into a slot still unset here (the C# "_colorArray[i]
        // == null && dataSet[i].IsSet" guard, so a lower pivot's value wins over a higher one as the resolve
        // walk moves up). Returns true if any slot was filled (the C# anySet).
        bool ingest(const element& pivot);

        // ShellAppearance.MakeComplete: a no-op here — the C# body only re-null-assigns already-null slots
        // (a defensive marker). Kept for surface fidelity; the resolve walk calls it after the ingest loop.
        void make_complete()
        {
        }

        // ShellAppearance.Equals / operator==: equal when every color slot and both doubles match.
        friend bool operator==(const shell_appearance& a, const shell_appearance& b)
        {
            return a.colors_ == b.colors_ && a.flyout_width_ == b.flyout_width_ && a.flyout_height_ == b.flyout_height_;
        }

    private:
        // The C# s_ingestArray order (the attached-property → slot mapping the ingest relies on).
        static constexpr std::size_t index_background = 0;
        static constexpr std::size_t index_disabled = 1;
        static constexpr std::size_t index_foreground = 2;
        static constexpr std::size_t index_tab_bar_background = 3;
        static constexpr std::size_t index_tab_bar_disabled = 4;
        static constexpr std::size_t index_tab_bar_foreground = 5;
        static constexpr std::size_t index_tab_bar_title = 6;
        static constexpr std::size_t index_tab_bar_unselected = 7;
        static constexpr std::size_t index_title = 8;
        static constexpr std::size_t index_unselected = 9;
        static constexpr std::size_t color_count = 10;

        std::array<std::optional<maui::graphics::color>, color_count> colors_{};
        std::optional<double> flyout_width_;
        std::optional<double> flyout_height_;
    };
} // namespace maui::controls
