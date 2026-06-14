// maui::controls::refresh_view — out-of-line definitions: the shared bindable-property descriptors (with
// the IsRefreshing coerce/changed + IsRefreshEnabled coerce/changed callbacks that mirror RefreshView.cs),
// the command channel (the ICommand collapse), Content + the layout, and the default handler
// self-registration. Ported from RefreshView.cs + RefreshViewTests.cs.

#include "maui/controls/refresh_view.hpp"

#include <algorithm>
#include <any>
#include <functional>
#include <memory>
#include <string_view>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/view.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_refresh_view.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/refresh_view_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/graphics/solid_paint.hpp"

namespace maui::controls
{
    // ---- shared bindable-property descriptors ----

    const maui::core::bindable_property<bool>& refresh_view::is_refreshing_property()
    {
        // C# RefreshView.IsRefreshingProperty: default false, TwoWay, coerceValue =
        // OnIsRefreshingPropertyCoerced, propertyChanged = OnIsRefreshingPropertyChanged.
        static const maui::core::bindable_property<bool> descriptor{
            "is_refreshing", false,
            maui::core::bindable_property<bool>::options{
                .property_changed =
                    [](maui::core::bindable_object& owner, const bool& /*old*/, const bool& value) {
                        // OnIsRefreshingPropertyChanged: when it lands true, raise Refreshing + run command.
                        if (!value)
                        {
                            return;
                        }
                        auto& self = dynamic_cast<refresh_view&>(owner);
                        self.refreshing.raise();
                        self.execute_command();
                    },
                .coerce_value = [](maui::core::bindable_object& owner, const bool& value) -> bool {
                    // OnIsRefreshingPropertyCoerced: false is always allowed; true requires IsEnabled
                    // AND IsRefreshEnabled (Command == null still allows the value).
                    auto& self = dynamic_cast<refresh_view&>(owner);
                    return self.coerce_is_refreshing(value);
                },
            }};
        return descriptor;
    }

    const maui::core::bindable_property<bool>& refresh_view::is_refresh_enabled_property()
    {
        // C# RefreshView.IsRefreshEnabledProperty: default true, coerceValue = CoerceIsRefreshEnabledProperty
        // (stores the explicit value, returns explicit && CanExecute(Command)), propertyChanged =
        // OnIsRefreshEnabledPropertyChanged (false while refreshing stops the refresh).
        static const maui::core::bindable_property<bool> descriptor{
            "is_refresh_enabled", true,
            maui::core::bindable_property<bool>::options{
                .property_changed =
                    [](maui::core::bindable_object& owner, const bool& /*old*/, const bool& value) {
                        if (value)
                        {
                            return;
                        }
                        auto& self = dynamic_cast<refresh_view&>(owner);
                        if (self.is_refreshing())
                        {
                            self.set_is_refreshing(false);
                        }
                    },
                .coerce_value = [](maui::core::bindable_object& owner, const bool& value) -> bool {
                    auto& self = dynamic_cast<refresh_view&>(owner);
                    // CoerceIsRefreshEnabledProperty: capture the explicit value (only when the caller
                    // is the developer, not the internal re-coerce that already set it), then AND with
                    // the command's CanExecute.
                    if (!self.coercing_refresh_enabled_)
                    {
                        self.is_refresh_enabled_explicit_ = value;
                    }
                    return self.is_refresh_enabled_explicit_ && self.command_can_execute();
                },
            }};
        return descriptor;
    }

    const maui::core::bindable_property<maui::graphics::color>& refresh_view::refresh_color_property()
    {
        // C# RefreshView.RefreshColorProperty default is null (no color).
        static const maui::core::bindable_property<maui::graphics::color> descriptor{"refresh_color"};
        return descriptor;
    }

    const maui::core::bindable_property<maui::core::thickness>& refresh_view::padding_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }

    // ---- IsRefreshing ----

    void refresh_view::set_is_refreshing(bool value)
    {
        is_refreshing_.set(value);
    }

    bool refresh_view::coerce_is_refreshing(bool requested) const
    {
        // C# OnIsRefreshingPropertyCoerced: false always allowed; true requires IsEnabled && IsRefreshEnabled.
        if (!requested)
        {
            return false;
        }
        if (!is_enabled() || !is_refresh_enabled())
        {
            return false;
        }
        return true; // Command == null OR present — both keep the value
    }

    // ---- IsRefreshEnabled ----

    void refresh_view::set_is_refresh_enabled(bool value)
    {
        is_refresh_enabled_.set(value);
    }

    void refresh_view::recoerce_is_refresh_enabled()
    {
        // C# RefreshView.CanExecuteChanged → RefreshPropertyValue(IsRefreshEnabledProperty, explicit):
        // re-push the explicit value so the coerce callback re-evaluates command_can_execute. Guard so the
        // coerce callback keeps the explicit value rather than overwriting it with the coerced result.
        coercing_refresh_enabled_ = true;
        is_refresh_enabled_.set(is_refresh_enabled_explicit_);
        coercing_refresh_enabled_ = false;
    }

    // ---- RefreshColor ----

    const maui::graphics::paint* refresh_view::refresh_color() const
    {
        // C# IRefreshView.RefreshColor => RefreshColor?.AsPaint(): null when no color set.
        if (!refresh_color_.is_set())
        {
            refresh_paint_.reset();
            return nullptr;
        }
        refresh_paint_ = std::make_unique<maui::graphics::solid_paint>(refresh_color_.get());
        return refresh_paint_.get();
    }

    // ---- the refresh command (the ICommand collapse) ----

    void refresh_view::set_command(std::function<void()> action, std::function<bool()> can_execute)
    {
        // C# CommandElement.OnCommandChanged: wiring (or clearing) the command re-coerces IsRefreshEnabled.
        action_ = std::move(action);
        can_execute_ = std::move(can_execute);
        action_with_param_ = nullptr;
        can_execute_with_param_ = nullptr;
        recoerce_is_refresh_enabled();
    }

    void refresh_view::set_command(std::function<void(const std::any&)> action,
                                   std::function<bool(const std::any&)> can_execute, std::any parameter)
    {
        action_with_param_ = std::move(action);
        can_execute_with_param_ = std::move(can_execute);
        action_ = nullptr;
        can_execute_ = nullptr;
        command_parameter_ = std::move(parameter);
        recoerce_is_refresh_enabled();
    }

    void refresh_view::set_command_parameter(std::any parameter)
    {
        // C# CommandElement.OnCommandParameterChanged: re-coerce (CanExecute depends on the parameter).
        command_parameter_ = std::move(parameter);
        recoerce_is_refresh_enabled();
    }

    void refresh_view::change_can_execute()
    {
        // C# ICommandElement.CanExecuteChanged: if already refreshing, do nothing (the C# early-out);
        // else re-coerce IsRefreshEnabled.
        if (is_refreshing())
        {
            return;
        }
        recoerce_is_refresh_enabled();
    }

    bool refresh_view::command_can_execute() const
    {
        // C# CommandElement.GetCanExecute: true when no command, else the predicate (default true).
        if (action_with_param_)
        {
            return !can_execute_with_param_ || can_execute_with_param_(command_parameter_);
        }
        if (action_)
        {
            return !can_execute_ || can_execute_();
        }
        return true;
    }

    void refresh_view::execute_command()
    {
        // C# Command?.Execute(CommandParameter): only when the command CanExecute (Command.Execute is a
        // no-op when CanExecute is false in the MAUI Command implementation).
        if (action_with_param_)
        {
            if (!can_execute_with_param_ || can_execute_with_param_(command_parameter_))
            {
                action_with_param_(command_parameter_);
            }
        }
        else if (action_)
        {
            if (!can_execute_ || can_execute_())
            {
                action_();
            }
        }
    }

    // ---- IsEnabled change → stop refreshing ----

    void refresh_view::on_property_changed(std::string_view name)
    {
        view<maui::core::i_refresh_view>::on_property_changed(name);
        // C# RefreshView.OnPropertyChanged: when IsEnabled becomes false, stop any active refresh.
        if (name == "is_enabled" && !is_enabled() && is_refreshing())
        {
            set_is_refreshing(false);
        }
    }

    // ---- Content ----

    void refresh_view::set_content(maui::core::i_view* value)
    {
        if (content_ == value)
        {
            return;
        }
        if (auto* old_child = dynamic_cast<element*>(content_))
        {
            detach_logical_child(*old_child);
        }
        content_ = value;
        if (auto* new_child = dynamic_cast<element*>(content_))
        {
            attach_logical_child(*new_child);
        }
        if (const auto& element_handler = handler())
        {
            element_handler->invoke("set_content");
        }
    }

    // ---- layout (the content_page MeasureContent / ArrangeContent recipe) ----

    maui::graphics::size refresh_view::measure(double width_constraint, double height_constraint)
    {
        const maui::core::thickness inset = padding();
        maui::graphics::size content_size{0, 0};
        if (content_ != nullptr)
        {
            content_size = content_->measure(width_constraint - inset.horizontal_thickness(),
                                             height_constraint - inset.vertical_thickness());
        }
        const maui::graphics::size measured{content_size.width + inset.horizontal_thickness(),
                                            content_size.height + inset.vertical_thickness()};
        desired_size_ = {resolve_size_request(measured.width, width(), minimum_width(), maximum_width()),
                         resolve_size_request(measured.height, height(), minimum_height(), maximum_height())};
        return desired_size_;
    }

    maui::graphics::size refresh_view::arrange(const maui::graphics::rect& bounds)
    {
        frame_ = bounds;
        if (content_ != nullptr)
        {
            const maui::core::thickness inset = padding();
            content_->arrange({bounds.x + inset.left, bounds.y + inset.top,
                               std::max(0.0, bounds.width - inset.horizontal_thickness()),
                               std::max(0.0, bounds.height - inset.vertical_thickness())});
        }
        if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler().get()))
        {
            view_handler->platform_arrange(bounds);
        }
        return {bounds.width, bounds.height};
    }
} // namespace maui::controls

// Self-register the default handler for refresh_view (opt-in, PROFILE §6).
MAUI_REGISTER_HANDLER(maui::controls::refresh_view, maui::core::refresh_view_handler)
