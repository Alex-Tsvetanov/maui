#pragma once
// maui::controls::text_cell  <=  Microsoft.Maui.Controls.TextCell
//
// A cell with primary Text + secondary Detail text (plus their colors) and a tap command. Ported from
// src/Controls/src/Core/Cells/TextCell.cs.
//
// Surface: Text / Detail (bindable strings), TextColor / DetailColor (bindable colors). OnTapped runs
// the command when IsEnabled (TextCell.OnTapped).
//
// Command (port convention, as button/check_box): C#'s ICommand collapses to the plain `command`
// callable plus an optional `command_can_execute` predicate + a `command_parameter`. Setting the pair
// drives IsEnabled from CanExecute (TextCell's ICommandElement.CanExecuteChanged → IsEnabled =
// CommandElement.GetCanExecute), refresh_can_execute() re-evaluates it (the ChangeCanExecute analog),
// and OnTapped executes `command` only when IsEnabled (TextCell.OnTapped guards on IsEnabled). The
// reflection-driven CanExecute auto-subscription is not ported — the developer calls
// refresh_can_execute() to re-poll, exactly the observable surface the C# ChangeCanExecute test drives.
//
// Color collapse (port convention): C#'s nullable Colors collapse to the non-nullable color value type.

#include <any>
#include <string>
#include <utility>

#include "maui/controls/cells/cell.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/graphics/color.hpp"

namespace maui::controls
{
    class text_cell : public cell
    {
    public:
        text_cell()
        {
            this->set_style_target_type<text_cell>();
        }

        // Shared bindable-property descriptors (one instance per type, like TextCell.*Property).
        static const maui::core::bindable_property<std::string>& text_property();
        static const maui::core::bindable_property<std::string>& detail_property();
        static const maui::core::bindable_property<maui::graphics::color>& text_color_property();
        static const maui::core::bindable_property<maui::graphics::color>& detail_color_property();

        // ---- Text / Detail ----
        [[nodiscard]] std::string text() const
        {
            return text_.get();
        }
        void set_text(std::string value)
        {
            text_.set(std::move(value));
        }
        [[nodiscard]] std::string detail() const
        {
            return detail_.get();
        }
        void set_detail(std::string value)
        {
            detail_.set(std::move(value));
        }

        // ---- TextColor / DetailColor ----
        [[nodiscard]] maui::graphics::color text_color() const
        {
            return text_color_.get();
        }
        void set_text_color(maui::graphics::color value)
        {
            text_color_.set(value);
        }
        [[nodiscard]] maui::graphics::color detail_color() const
        {
            return detail_color_.get();
        }
        void set_detail_color(maui::graphics::color value)
        {
            detail_color_.set(value);
        }

        // ---- command (the tap channel; see header note) ----
        maui::core::move_only_function<void()> command;
        maui::core::move_only_function<bool()> command_can_execute;
        std::any command_parameter; // TextCell.CommandParameter (passed to the command; opaque here)

        // Wire (or rewire) the command + optional CanExecute predicate. Immediately drives IsEnabled
        // from CanExecute (TextCell's CommandElement.OnCommandChanged path runs CanExecute on assign).
        void set_command(maui::core::move_only_function<void()> value,
                         maui::core::move_only_function<bool()> can_execute = {})
        {
            command = std::move(value);
            command_can_execute = std::move(can_execute);
            refresh_can_execute();
        }
        // ICommand.ChangeCanExecute → ICommandElement.CanExecuteChanged: re-poll CanExecute into IsEnabled.
        void refresh_can_execute()
        {
            if (command && command_can_execute)
            {
                set_is_enabled(command_can_execute());
            }
        }

        // TextCell.OnTapped: base tap, then (when enabled) execute the command.
        void on_tapped() override
        {
            cell::on_tapped();
            if (!is_enabled())
            {
                return;
            }
            if (command)
            {
                command();
            }
        }

    private:
        maui::core::property<std::string> text_{*this, text_property()};
        maui::core::property<std::string> detail_{*this, detail_property()};
        maui::core::property<maui::graphics::color> text_color_{*this, text_color_property()};
        maui::core::property<maui::graphics::color> detail_color_{*this, detail_color_property()};
    };
} // namespace maui::controls
