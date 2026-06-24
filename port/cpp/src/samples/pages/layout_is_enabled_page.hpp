#pragma once
// maui::samples::layout_is_enabled_page — ports LayoutIsEnabledPage.xaml (+ LayoutIsEnabledPage.xaml.cs)
//
// The C# page demonstrates how IsEnabled on a layout cascades to its children: a 2x2 grid whose
// left column hosts a "MainLayout" full of state-demo sub-stacks (all-enabled / all-disabled / disabled
// -because-the-layout-is-disabled / mixed / command-bound / nested), two control buttons below it
// ("Disable Layout" toggles MainLayout.IsEnabled, "Enable Button" toggles two specific buttons), and a
// right column whose top layout's IsEnabled is data-bound to IsLayoutEnabled plus three checkboxes
// binding IsLayoutEnabled / IsButtonEnabled / IsCommandEnabled.
//
// PORT MAPPING:
//   - IsEnabled cascading from a layout to its children  -> view::set_is_enabled / is_enabled
//     (controls/view.hpp; layout derives view). Setting the parent stack's IsEnabled to false is the
//     thing on display; the framework propagates the effective-enabled state to children.
//   - the ScrollView > Grid (ColumnDefinitions="*,*" RowDefinitions="Auto,Auto")  -> scroll_view over a
//     grid with two star columns + two rows (controls/grid.hpp), children placed with set_row/set_column.
//   - Button Clicked="OnDisableLayoutBtnClicked"  -> button::command flipping main_layout_.is_enabled
//     and rewriting the button text (the C# OnDisableLayoutBtnClicked, incl. the text ternary).
//   - Button Clicked="OnDisableButtonBtnClicked"  -> button::command flipping the two named buttons'
//     is_enabled + rewriting their text and the control button's text (OnDisableButtonBtnClicked).
//   - CheckBox IsChecked="{Binding IsLayoutEnabled/IsButtonEnabled}"  -> checkbox::checked_changed
//     handlers driving right_layout_.is_enabled and right_mixed_button_.is_enabled directly (the
//     code-first stand-in for the two-way bindings, which the prompt permits).
//   - Button Command="{Binding TheCommand}" + CanExecute(IsCommandEnabled)  -> button::command plus a
//     gate flag the "Enable/Disable Command" checkbox flips (Command.ChangeCanExecute → here a guarded
//     command that no-ops while the gate is false). note: the full ICommand CanExecute → IsEnabled
//     visual coupling lives on the binding layer; modelled here as a guard flag.
//
// HEADLESS-SAFE maui:: API only; the page owns its whole element tree and attaches every owned view
// bottom-up (the value_controls_page / shapes_page convention). Sub-stacks + their buttons are co-owned
// via shared_ptr so the whole tree stays alive and reachable for handler attachment.
//
// note: the implicit Style targeting VerticalStackLayout (Padding=6) and the Grid Row/Column placement
//       are applied where the headless surface allows (padding on each stack; set_row/set_column on the
//       grid). The Background colors per sub-stack are decorative — set_background applied where the
//       value is meaningful, omitted otherwise.

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/button.hpp"
#include "maui/controls/check_box.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/grid.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/scroll_view.hpp"
#include "maui/controls/vertical_stack_layout.hpp"
#include "maui/core/grid_length.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::samples
{
    class layout_is_enabled_page
    {
    public:
        layout_is_enabled_page()
        {
            page_.set_title("Layout IsEnabled");

            grid_.set_column_spacing(6);
            grid_.set_row_spacing(6);
            grid_.add_column_definition(maui::core::grid_length::star());
            grid_.add_column_definition(maui::core::grid_length::star());
            grid_.add_row_definition(maui::core::grid_length::automatic());
            grid_.add_row_definition(maui::core::grid_length::automatic());

            build_left_column();
            build_left_controls();
            build_right_column();
            build_right_controls();

            scroller_.set_content(grid_);
            page_.set_content(scroller_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned controls, exposed for the hosting main's inspection / tests.
        [[nodiscard]] maui::controls::grid& root_grid()
        {
            return grid_;
        }
        [[nodiscard]] maui::controls::vertical_stack_layout& main_layout()
        {
            return main_layout_;
        }
        [[nodiscard]] maui::controls::vertical_stack_layout& right_layout()
        {
            return right_layout_;
        }
        [[nodiscard]] maui::controls::button& disable_layout_button()
        {
            return disable_layout_button_;
        }
        [[nodiscard]] maui::controls::check_box& layout_check()
        {
            return layout_check_;
        }

    private:
        // ---- Left column: MainLayout with the six state-demo sub-stacks ----
        void build_left_column()
        {
            main_layout_.set_padding(maui::core::thickness(6)); // the implicit VSL Style Padding="6".
            main_layout_.set_spacing(6);

            add_caption(main_layout_, "All children are enabled");
            add_sub_stack(main_layout_, /*enabled=*/{true, true}, light_blue());

            add_caption(main_layout_, "All children are disabled");
            add_sub_stack(main_layout_, {false, false}, light_blue());

            add_caption(main_layout_, "All children are disabled (because layout is disabled)");
            {
                auto* sub = add_sub_stack(main_layout_, {true, true}, light_pink());
                sub->set_is_enabled(false); // VerticalStackLayout IsEnabled="False" — cascades to kids.
            }

            add_caption(main_layout_, "First item is enabled and the second one is disabled");
            add_sub_stack(main_layout_, {true, false}, light_sea_green());
            disabled_button_ = sub_last_button_; // x:Name="DisabledButton" (toggled by the control).

            add_caption(main_layout_, "Children have commands attached");
            add_command_sub_stack(main_layout_, {true, false});
            disabled_command_button_ = sub_last_button_; // x:Name="DisabledCommandButton".

            add_caption(main_layout_, "Nested layouts");
            add_nested_sub_stack(main_layout_);

            grid_.add(main_layout_);
            grid_.set_row(main_layout_, 0);
            grid_.set_column(main_layout_, 0);
        }

        void build_left_controls()
        {
            left_controls_.set_padding(maui::core::thickness(6));
            left_controls_.set_spacing(6);

            disable_layout_button_.set_text("Disable Layout");
            disable_layout_button_.command = [this] {
                const bool now_enabled = !main_layout_.is_enabled();
                main_layout_.set_is_enabled(now_enabled);
                disable_layout_button_.set_text(now_enabled ? "Disable Layout" : "Enable Layout");
            };

            enable_button_button_.set_text("Enable Button");
            enable_button_button_.command = [this] {
                if (disabled_button_ == nullptr || disabled_command_button_ == nullptr)
                {
                    return;
                }
                const bool now_enabled = !disabled_button_->is_enabled();
                disabled_button_->set_is_enabled(now_enabled);
                disabled_command_button_->set_is_enabled(now_enabled);
                disabled_button_->set_text(now_enabled ? "Enabled" : "Disabled");
                disabled_command_button_->set_text(now_enabled ? "Enabled" : "Disabled");
                enable_button_button_.set_text(now_enabled ? "Disable Button" : "Enable Button");
            };

            left_controls_.add(disable_layout_button_);
            left_controls_.add(enable_button_button_);

            grid_.add(left_controls_);
            grid_.set_row(left_controls_, 1);
            grid_.set_column(left_controls_, 0);
        }

        // ---- Right column: a layout whose IsEnabled is "bound" to IsLayoutEnabled ----
        void build_right_column()
        {
            right_layout_.set_padding(maui::core::thickness(6));
            right_layout_.set_spacing(6);
            right_layout_.set_is_enabled(false); // IsEnabled="{Binding IsLayoutEnabled}" starts false.

            add_caption(right_layout_, "All children are enabled");
            add_sub_stack(right_layout_, {true, true}, light_blue());

            add_caption(right_layout_, "All children are disabled");
            add_sub_stack(right_layout_, {false, false}, light_blue());

            add_caption(right_layout_, "All children are disabled (because layout is disabled)");
            {
                auto* sub = add_sub_stack(right_layout_, {true, true}, light_pink());
                sub->set_is_enabled(false);
            }

            add_caption(right_layout_, "First item is enabled and the second one is disabled");
            add_sub_stack(right_layout_, {true, false}, light_sea_green());
            right_mixed_button_ = sub_last_button_; // IsEnabled="{Binding IsButtonEnabled}".

            add_caption(right_layout_, "Children have commands attached");
            add_command_sub_stack(right_layout_, {true, false});

            add_caption(right_layout_, "Nested layouts");
            add_nested_sub_stack(right_layout_);

            grid_.add(right_layout_);
            grid_.set_row(right_layout_, 0);
            grid_.set_column(right_layout_, 1);
        }

        void build_right_controls()
        {
            right_controls_.set_padding(maui::core::thickness(6));
            right_controls_.set_spacing(6);

            add_caption(right_controls_, "Enable/Disable Layout");
            // CheckBox IsChecked="{Binding IsLayoutEnabled}" — drive the right layout's IsEnabled.
            layout_check_.checked_changed.connect([this](bool checked) { right_layout_.set_is_enabled(checked); });

            add_caption(right_controls_, "Enable/Disable Button");
            // CheckBox IsChecked="{Binding IsButtonEnabled}" — drive the mixed button's IsEnabled.
            button_check_.checked_changed.connect([this](bool checked) {
                if (right_mixed_button_ != nullptr)
                {
                    right_mixed_button_->set_is_enabled(checked);
                }
            });

            add_caption(right_controls_, "Enable/Disable Command");
            // CheckBox IsChecked="{Binding IsCommandEnabled}" — gate the bound command's CanExecute.
            command_check_.checked_changed.connect([this](bool checked) { command_enabled_ = checked; });

            right_controls_.add(layout_check_);
            right_controls_.add(button_check_);
            right_controls_.add(command_check_);

            grid_.add(right_controls_);
            grid_.set_row(right_controls_, 1);
            grid_.set_column(right_controls_, 1);
        }

        // ---- builders (each co-owns the views it mints) ----
        void add_caption(maui::controls::vertical_stack_layout& parent, const std::string& text)
        {
            auto caption = std::make_shared<maui::controls::label>();
            caption->set_text(text);
            parent.add(*caption);
            caption_labels_.push_back(std::move(caption));
        }

        // A two-button sub-stack with the given per-button enabled flags + a background; records the LAST
        // button in sub_last_button_ so the caller can name it (DisabledButton / the mixed button).
        maui::controls::vertical_stack_layout* add_sub_stack(maui::controls::vertical_stack_layout& parent,
                                                             std::array<bool, 2> enabled,
                                                             const maui::graphics::color& background)
        {
            auto sub = std::make_shared<maui::controls::vertical_stack_layout>();
            sub->set_padding(maui::core::thickness(6));
            sub->set_background(std::make_shared<maui::graphics::solid_paint>(background));
            for (std::size_t i = 0; i < enabled.size(); ++i)
            {
                auto button = std::make_shared<maui::controls::button>();
                button->set_text(enabled[i] ? "Enabled" : "Disabled");
                button->set_is_enabled(enabled[i]);
                sub->add(*button);
                sub_last_button_ = button.get();
                buttons_.push_back(std::move(button));
            }
            parent.add(*sub);
            auto* raw = sub.get();
            sub_stacks_.push_back(std::move(sub));
            return raw;
        }

        // Like add_sub_stack but every button carries the shared "TheCommand" (guarded by command_enabled_).
        maui::controls::vertical_stack_layout* add_command_sub_stack(maui::controls::vertical_stack_layout& parent,
                                                                     std::array<bool, 2> enabled)
        {
            auto sub = std::make_shared<maui::controls::vertical_stack_layout>();
            sub->set_padding(maui::core::thickness(6));
            sub->set_background(std::make_shared<maui::graphics::solid_paint>(light_sea_green()));
            for (std::size_t i = 0; i < enabled.size(); ++i)
            {
                auto button = std::make_shared<maui::controls::button>();
                button->set_text(enabled[i] ? "Enabled" : "Disabled");
                button->set_is_enabled(enabled[i]);
                // Button Command="{Binding TheCommand}" — guarded by the CanExecute gate (OnTheCanExecute).
                button->command = [this] {
                    if (command_enabled_)
                    {
                        ++command_invocations_; // stands in for OnThe()'s Debug.WriteLine side effect.
                    }
                };
                sub->add(*button);
                sub_last_button_ = button.get();
                buttons_.push_back(std::move(button));
            }
            parent.add(*sub);
            auto* raw = sub.get();
            sub_stacks_.push_back(std::move(sub));
            return raw;
        }

        // The "Nested layouts" demo: a sub-stack containing an inner sub-stack with two buttons.
        void add_nested_sub_stack(maui::controls::vertical_stack_layout& parent)
        {
            auto outer = std::make_shared<maui::controls::vertical_stack_layout>();
            outer->set_padding(maui::core::thickness(6));
            outer->set_background(std::make_shared<maui::graphics::solid_paint>(light_sky_blue()));

            auto inner = std::make_shared<maui::controls::vertical_stack_layout>();
            inner->set_padding(maui::core::thickness(6));
            inner->set_background(std::make_shared<maui::graphics::solid_paint>(light_gray()));

            auto enabled_button = std::make_shared<maui::controls::button>();
            enabled_button->set_text("Enabled");
            auto disabled_button = std::make_shared<maui::controls::button>();
            disabled_button->set_text("Disabled");
            disabled_button->set_is_enabled(false);

            inner->add(*enabled_button);
            inner->add(*disabled_button);
            outer->add(*inner);
            parent.add(*outer);

            buttons_.push_back(std::move(enabled_button));
            buttons_.push_back(std::move(disabled_button));
            sub_stacks_.push_back(std::move(inner));
            sub_stacks_.push_back(std::move(outer));
        }

        // The sub-stack backgrounds (XAML named colors).
        static maui::graphics::color light_blue()
        {
            return maui::graphics::color::from_rgb(173, 216, 230); // LightBlue #ADD8E6
        }
        static maui::graphics::color light_pink()
        {
            return maui::graphics::color::from_rgb(255, 182, 193); // LightPink #FFB6C1
        }
        static maui::graphics::color light_sea_green()
        {
            return maui::graphics::color::from_rgb(32, 178, 170); // LightSeaGreen #20B2AA
        }
        static maui::graphics::color light_sky_blue()
        {
            return maui::graphics::color::from_rgb(135, 206, 250); // LightSkyBlue #87CEFA
        }
        static maui::graphics::color light_gray()
        {
            return maui::graphics::color::from_rgb(211, 211, 211); // LightGray #D3D3D3
        }

        maui::controls::content_page page_;
        maui::controls::scroll_view scroller_;
        maui::controls::grid grid_;

        maui::controls::vertical_stack_layout main_layout_;    // left column top (x:Name="MainLayout")
        maui::controls::vertical_stack_layout left_controls_;  // left column bottom (the two buttons)
        maui::controls::vertical_stack_layout right_layout_;   // right column top (bound IsEnabled)
        maui::controls::vertical_stack_layout right_controls_; // right column bottom (the checkboxes)

        maui::controls::button disable_layout_button_;
        maui::controls::button enable_button_button_;
        maui::controls::check_box layout_check_;
        maui::controls::check_box button_check_;
        maui::controls::check_box command_check_;

        // Named buttons the control buttons / checkboxes drive (borrowed from leaves_).
        maui::controls::button* disabled_button_ = nullptr;
        maui::controls::button* disabled_command_button_ = nullptr;
        maui::controls::button* right_mixed_button_ = nullptr;
        maui::controls::button* sub_last_button_ = nullptr; // scratch: the last button a builder minted

        bool command_enabled_ = false; // the IsCommandEnabled gate (OnTheCanExecute).
        int command_invocations_ = 0;  // OnThe() side-effect stand-in.

        // The co-owned leaves, kept alive + reachable for bottom-up handler attachment. Stored in
        // TYPE-CONCRETE vectors (never erased to a base) so the generic `one(...)` attach lambda sees
        // each view's real static type — erasing to element&/i_view& blanks the page (the generic mount
        // needs the concrete view type).
        std::vector<std::shared_ptr<maui::controls::label>> caption_labels_;
        std::vector<std::shared_ptr<maui::controls::button>> buttons_;
        std::vector<std::shared_ptr<maui::controls::vertical_stack_layout>> sub_stacks_;
    };
} // namespace maui::samples
