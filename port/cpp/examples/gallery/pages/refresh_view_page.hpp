#pragma once
// maui::samples::refresh_view_page — ports RefreshViewPage.xaml (+ RefreshViewPage.xaml.cs +
// RefreshViewModel.cs)
//
// A self-contained, code-first demo page for the RefreshView control (the C# gallery-page convention,
// mirroring the swipe_refresh_page pattern). The page OWNS its whole element tree; `page()` hands back
// the content_page; the generic mount (app_host.hpp) attaches every owned view's handler and hosts the tree
// (leaves → layout → page) through a guarded try/catch, then re-hosts the tree.
//
// The C# page pairs a RefreshView (wrapping a scrolled, bindable item collection) with a column of
// toggle buttons and status labels, driven by RefreshViewModel. This port keeps the cross-platform
// RefreshView API and wires the demonstrated INTERACTIONS:
//   - The model seeds 50 items; the refresh Command adds another 50 each refresh and the count readout
//     (inside the RefreshView's content) updates (the C# RefreshItemsAsync → AddItems; the async
//     Task.Delay collapses to a synchronous bump — headless-safe).
//   - The Command's CanExecute is `!IsRefreshing` (the C# `() => !IsRefreshing`), wired through
//     set_command(action, can_execute) so IsRefreshEnabled coerces off while a refresh is in flight.
//   - "Toggle Refresh" flips IsRefreshing (the C# OnTriggerRefreshClicked); landing true raises
//     `refreshing` and runs the command.
//   - "Toggle Is Enabled" flips IsEnabled (the C# OnToggleEnabledClicked); disabling while refreshing
//     stops the refresh (RefreshView coercion).
//   - "Toggle Refresh Color" swaps the spinner color Teal↔Red (the C# OnToggleRefreshColorClicked).
//   - "Toggle Background Color" swaps the RefreshView background Yellow↔Green (the C#
//     OnToggleRefreshBackgroundColorClicked).
//   - RefreshText / EnabledText status labels echo IsRefreshing / IsEnabled (the C# bindings).
//
// Faithful best-effort deviations (// note:):
//   - C#'s BindableLayout color-box grid (DataTemplate over Items) is markup/template-era surface; the
//     port renders the item COUNT into a label inside the RefreshView content instead of a templated
//     box grid (the count is the observable behavior the demo turns on).
//   - The C# 2-second async refresh collapses to a synchronous AddItems + IsRefreshing=false (the
//     swipe_refresh_page convention: the gallery ends the spinner inside the command).

#include <string>

#include "maui/controls/brushes/solid_color_brush.hpp"
#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/horizontal_stack_layout.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/refresh_view.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/colors.hpp"

namespace maui::samples
{
    class refresh_view_page
    {
    public:
        refresh_view_page()
        {
            page_.set_title("RefreshView");

            // The top control column (header + toggles + status), the C# Grid.Row=0 stack.
            controls_stack_.set_spacing(6);
            controls_stack_.set_padding(maui::core::thickness(12));

            header_.set_text("Pull the items down to refresh the ScrollView.");
            item_count_label_.set_text(make_count_text());

            // Toggle-color + toggle-background row (the C# first HorizontalStackLayout).
            color_buttons_.set_spacing(6);
            toggle_refresh_color_button_.set_text("Toggle Refresh Color");
            toggle_refresh_color_button_.clicked.connect([this] { toggle_refresh_color(); });
            toggle_background_button_.set_text("Toggle Background Color");
            toggle_background_button_.clicked.connect([this] { toggle_background_color(); });
            color_buttons_.add(toggle_refresh_color_button_);
            color_buttons_.add(toggle_background_button_);

            // Toggle-refresh + toggle-enabled row (the C# second HorizontalStackLayout).
            toggle_buttons_.set_spacing(6);
            toggle_refresh_button_.set_text("Toggle Refresh");
            toggle_refresh_button_.clicked.connect([this] { toggle_refresh(); });
            toggle_enabled_button_.set_text("Toggle Is Enabled");
            toggle_enabled_button_.clicked.connect([this] { toggle_enabled(); });
            toggle_buttons_.add(toggle_refresh_button_);
            toggle_buttons_.add(toggle_enabled_button_);

            // Status labels (the C# RefreshText / EnabledText bindings).
            refresh_text_.set_text(make_refresh_text());
            enabled_text_.set_text(make_enabled_text());

            controls_stack_.add(header_);
            controls_stack_.add(item_count_label_);
            controls_stack_.add(color_buttons_);
            controls_stack_.add(toggle_buttons_);
            controls_stack_.add(refresh_text_);
            controls_stack_.add(enabled_text_);

            // The RefreshView wrapping a scrolled content readout (the C# Grid.Row=1 RefreshView).
            refresh_content_.set_text("50 items loaded. Pull to add more.");
            inner_scroll_.set_content(refresh_content_);
            refresh_.set_refresh_color(maui::graphics::colors::teal);
            // The refresh command: add 50 items, refresh the readouts, end the spinner. CanExecute is
            // `!IsRefreshing` (the C# RefreshCommand predicate) so IsRefreshEnabled coerces correctly.
            refresh_.set_command([this] { run_refresh(); }, [this] { return !refresh_.is_refreshing(); });
            refresh_.set_content(inner_scroll_);

            // The page body grid: row 0 = controls, row 1 = the RefreshView (the C# two-row Grid).
            body_.add_row_definition(maui::core::grid_length::automatic());
            body_.add_row_definition(maui::core::grid_length::star());
            body_.add(controls_stack_);
            body_.set_row(controls_stack_, 0);
            body_.add(refresh_);
            body_.set_row(refresh_, 1);

            page_.set_content(body_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's bottom-up handler attachment + the tests.
        [[nodiscard]] maui::controls::grid& body()
        {
            return body_;
        }
        [[nodiscard]] maui::controls::refresh_view& refresh()
        {
            return refresh_;
        }
        [[nodiscard]] maui::controls::label& refresh_text()
        {
            return refresh_text_;
        }
        [[nodiscard]] maui::controls::label& enabled_text()
        {
            return enabled_text_;
        }
        [[nodiscard]] int item_count() const
        {
            return item_count_;
        }

    private:
        void run_refresh()
        {
            item_count_ += 50; // the C# AddItems loop (50 items)
            item_count_label_.set_text(make_count_text());
            refresh_content_.set_text(std::to_string(item_count_) + " items loaded. Pull to add more.");
            refresh_.set_is_refreshing(false); // end the spinner (the gallery convention)
            sync_status();
        }

        void toggle_refresh()
        {
            refresh_.set_is_refreshing(!refresh_.is_refreshing());
            sync_status();
        }

        void toggle_enabled()
        {
            refresh_.set_is_enabled(!refresh_.is_enabled());
            sync_status();
        }

        void toggle_refresh_color()
        {
            refresh_color_is_teal_ = !refresh_color_is_teal_;
            refresh_.set_refresh_color(refresh_color_is_teal_ ? maui::graphics::colors::teal
                                                              : maui::graphics::colors::red);
        }

        void toggle_background_color()
        {
            background_is_yellow_ = !background_is_yellow_;
            refresh_.set_background_brush(std::make_shared<maui::controls::solid_color_brush>(
                background_is_yellow_ ? maui::graphics::colors::yellow : maui::graphics::colors::green));
        }

        void sync_status()
        {
            refresh_text_.set_text(make_refresh_text());
            enabled_text_.set_text(make_enabled_text());
        }

        [[nodiscard]] std::string make_count_text() const
        {
            return "Number of items: " + std::to_string(item_count_);
        }
        [[nodiscard]] std::string make_refresh_text() const
        {
            return std::string("Is Refreshing: ") + (refresh_.is_refreshing() ? "True" : "False");
        }
        [[nodiscard]] std::string make_enabled_text() const
        {
            return std::string("Is Enabled: ") + (refresh_.is_enabled() ? "True" : "False");
        }

        maui::controls::content_page page_;
        maui::controls::grid body_;
        maui::controls::vertical_stack_layout controls_stack_;

        maui::controls::label header_;
        maui::controls::label item_count_label_;
        maui::controls::horizontal_stack_layout color_buttons_;
        maui::controls::button toggle_refresh_color_button_;
        maui::controls::button toggle_background_button_;
        maui::controls::horizontal_stack_layout toggle_buttons_;
        maui::controls::button toggle_refresh_button_;
        maui::controls::button toggle_enabled_button_;
        maui::controls::label refresh_text_;
        maui::controls::label enabled_text_;

        maui::controls::refresh_view refresh_;
        maui::controls::scroll_view inner_scroll_;
        maui::controls::label refresh_content_;

        int item_count_ = 50; // the C# RefreshViewModel seeds 50 items
        bool refresh_color_is_teal_ = true;
        bool background_is_yellow_ = false;
    };
} // namespace maui::samples
