#pragma once
// maui::controls::refresh_view  <=  Microsoft.Maui.Controls.RefreshView
//
// A container providing pull-to-refresh over scrollable content. Ported from
// src/Controls/src/Core/RefreshView/RefreshView.cs (RefreshView : ContentView, IRefreshView,
// ICommandElement): Content + IsRefreshing + RefreshColor + IsRefreshEnabled + a refresh Command, with
// the rich coercion the RefreshViewTests pin:
//   - IsRefreshing (default false): toggling to true is COERCED to false unless IsEnabled AND
//     IsRefreshEnabled (OnIsRefreshingPropertyCoerced). When it lands true, `refreshing` is raised and the
//     command (if any) executes (OnIsRefreshingPropertyChanged). Toggling to false is always allowed.
//   - IsRefreshEnabled (default true): the EXPLICIT value AND the command's CanExecute
//     (CoerceIsRefreshEnabledProperty stores the explicit value, returns explicit && CanExecute(Command)).
//     Setting it false while refreshing stops the refresh (OnIsRefreshEnabledPropertyChanged).
//   - Setting IsEnabled false while refreshing stops the refresh (OnPropertyChanged(IsEnabled)).
//   - The command's CanExecute changing re-coerces IsRefreshEnabled (ICommandElement.CanExecuteChanged) —
//     UNLESS already refreshing (the early-out C# keeps).
//   - RefreshColor → the spinner color (AsPaint); RefreshColor() returns the paint the handler reads.
//
// COMMAND MODEL: the port has no ICommand (the command channel collapses to a callback + an optional
// CanExecute predicate, per the W1-11 convention). set_command(action, can_execute) wires both;
// change_can_execute() is the Command.ChangeCanExecute() the tests call to flip CanExecute and re-coerce.
// command_parameter() participates in CanExecute(parameter) (CommandPropertyChangesIsRefreshEnabled).
//
// API shape: bare-noun i_refresh_view getters + method accessors over private property<T> engines, the
// port convention. Content is a non-owning child pointer (content_page recipe).
//
// DEVIATION (documented): C#'s ControlTemplate hosting is not ported (the templated_view base is
// incompatible with the i_refresh_view contract diamond — same as swipe_view). Content BindingContext
// propagation is preserved via the direct content-host recipe.

#include <any>
#include <functional>
#include <memory>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/event.hpp"
#include "maui/core/i_element_handler.hpp"
#include "maui/core/i_refresh_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/property.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::controls
{
    class refresh_view : public view<maui::core::i_refresh_view>
    {
    public:
        refresh_view()
        {
            this->set_style_target_type<refresh_view>();
        }

        // Shared bindable-property descriptors (RefreshView.IsRefreshing/IsRefreshEnabled/RefreshColor +
        // the page Padding).
        static const maui::core::bindable_property<bool>& is_refreshing_property();
        static const maui::core::bindable_property<bool>& is_refresh_enabled_property();
        static const maui::core::bindable_property<maui::graphics::color>& refresh_color_property();
        static const maui::core::bindable_property<maui::core::thickness>& padding_property();

        // C# RefreshView.Refreshing — raised when IsRefreshing lands true.
        maui::core::event<> refreshing;

        // ---- Content (non-owning child; the content_page recipe) ----
        [[nodiscard]] maui::core::i_view* content() const override
        {
            return content_;
        }
        void set_content(maui::core::i_view& value)
        {
            set_content(&value);
        }
        void set_content(maui::core::i_view* value);

        // ---- Padding (control-level: C#'s RefreshView Padding comes from VisualElement, NOT the
        // IRefreshView contract — IRefreshView : IView has no IPadding, so this is not an override) ----
        [[nodiscard]] maui::core::thickness padding() const
        {
            return padding_.get();
        }
        void set_padding(maui::core::thickness value)
        {
            padding_.set(value);
        }

        // ---- IsRefreshing (coerced; see header) ----
        [[nodiscard]] bool is_refreshing() const override
        {
            return is_refreshing_.get();
        }
        void set_is_refreshing(bool value) override;

        // ---- IsRefreshEnabled (explicit AND command CanExecute; see header) ----
        [[nodiscard]] bool is_refresh_enabled() const override
        {
            return is_refresh_enabled_.get();
        }
        void set_is_refresh_enabled(bool value);

        // ---- RefreshColor ----
        [[nodiscard]] maui::graphics::color refresh_color_value() const
        {
            return refresh_color_.get();
        }
        [[nodiscard]] bool has_refresh_color() const
        {
            return refresh_color_.is_set();
        }
        void set_refresh_color(maui::graphics::color value)
        {
            refresh_color_.set(value);
        }
        // C# IRefreshView.RefreshColor => RefreshColor?.AsPaint(): the spinner paint the handler reads
        // (null when no color is set). Materialized lazily so the borrowed pointer stays valid.
        [[nodiscard]] const maui::graphics::paint* refresh_color() const override;

        // ---- the refresh command (the port's ICommand collapse — see header) ----
        // Wire the refresh command: `action` runs when a refresh lands; `can_execute` (optional) gates
        // IsRefreshEnabled. Passing an empty `action` clears the command (RemovedCommandEnablesRefreshView).
        void set_command(std::function<void()> action, std::function<bool()> can_execute = {});
        // C# RefreshView.CommandParameter (participates in CanExecute(parameter)). A parameterized
        // CanExecute predicate variant.
        void set_command(std::function<void(const std::any&)> action, std::function<bool(const std::any&)> can_execute,
                         std::any parameter = {});
        void set_command_parameter(std::any parameter);
        // C# Command.ChangeCanExecute(): re-evaluate CanExecute and re-coerce IsRefreshEnabled
        // (ICommandElement.CanExecuteChanged).
        void change_can_execute();

        // ---- layout pass: MeasureContent / ArrangeContent within the padding (content_page recipe) ----
        maui::graphics::size measure(double width_constraint, double height_constraint) override;
        maui::graphics::size arrange(const maui::graphics::rect& bounds) override;

    protected:
        void for_each_logical_child(const std::function<void(element&)>& visit) const override
        {
            if (auto* child = dynamic_cast<element*>(content_))
            {
                visit(*child);
            }
        }

        // C# RefreshView.OnPropertyChanged: when IsEnabled becomes false while refreshing, stop the
        // refresh. (The IsRefreshing/IsRefreshEnabled coercion runs through the descriptor callbacks.)
        // Protected to match the bindable_object base.
        void on_property_changed(std::string_view name) override;

    private:
        // Whether a command is wired (C# Command != null).
        [[nodiscard]] bool has_command() const
        {
            return static_cast<bool>(action_) || static_cast<bool>(action_with_param_);
        }
        // C# CommandElement.GetCanExecute(this, CommandProperty): true when no command, else the
        // predicate's verdict (default true if a command has no predicate).
        [[nodiscard]] bool command_can_execute() const;
        // Run the command (C# Command.Execute(parameter)).
        void execute_command();
        // Re-coerce IsRefreshEnabled to (explicit && command_can_execute) and push the result.
        void recoerce_is_refresh_enabled();
        // The descriptor coerce/changed callbacks (static, downcast the bindable_object — see the .cpp).
        // Declared here as the IsRefreshing/IsRefreshEnabled coercion needs the instance state.
        [[nodiscard]] bool coerce_is_refreshing(bool requested) const;

        maui::core::i_view* content_ = nullptr; // NON-owning: the caller owns the content's lifetime
        // The refresh command (the ICommand collapse). Either the bare or the parameterized form is set.
        std::function<void()> action_;
        std::function<bool()> can_execute_;
        std::function<void(const std::any&)> action_with_param_;
        std::function<bool(const std::any&)> can_execute_with_param_;
        std::any command_parameter_;
        // C# RefreshView._isRefreshEnabledExplicit — the developer-set IsRefreshEnabled before the
        // command-CanExecute coercion. Default true (IsRefreshEnabledProperty.DefaultValue).
        bool is_refresh_enabled_explicit_ = true;
        // Guards reentrancy while coercing IsRefreshEnabled (the C# RefreshPropertyValue path).
        bool coercing_refresh_enabled_ = false;

        maui::core::property<bool> is_refreshing_{*this, is_refreshing_property()};
        maui::core::property<bool> is_refresh_enabled_{*this, is_refresh_enabled_property()};
        maui::core::property<maui::graphics::color> refresh_color_{*this, refresh_color_property()};
        maui::core::property<maui::core::thickness> padding_{*this, padding_property()};
        // Lazily-built SolidPaint backing the borrowed refresh_color() pointer.
        mutable std::unique_ptr<maui::graphics::solid_paint> refresh_paint_;
    };
} // namespace maui::controls
