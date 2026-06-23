// maui::controls::content_presenter — the TemplatedParent.Content pull, the content transition, and
// the layout pass (content_presenter.hpp). Ported from ContentPresenter.cs (+ the OnContentChanged
// logical re-parenting).
#include "maui/controls/templates/content_presenter.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "maui/controls/templates/i_control_templated.hpp"
#include "maui/controls/templates/template_binding.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/content_page_handler.hpp"
#include "maui/core/event.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
#include "maui/core/thickness.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

namespace maui::controls
{
    content_presenter::content_presenter()
    {
        this->set_style_target_type<content_presenter>();
        // The C# ctor's SetBinding(ContentProperty, (IContentView v) => v.Content, source:
        // RelativeBindingSource.TemplatedParent): whenever this presenter's template scope resolves
        // (or dissolves), re-present the templated parent's developer content. Later Content changes
        // on the parent are pushed by template_utilities::on_content_changed.
        set_template_binding(template_binding{
            "content", [](maui::core::bindable_object& target, maui::core::bindable_object* templated_parent,
                          std::vector<maui::core::scoped_connection>& /*connections*/) {
                auto& self = dynamic_cast<content_presenter&>(target);
                const auto* templated = dynamic_cast<const i_control_templated*>(templated_parent);
                self.set_content(templated != nullptr ? templated->templated_content() : nullptr);
            }});
    }

    const maui::core::bindable_property<maui::core::thickness>& content_presenter::padding_property()
    {
        static const maui::core::bindable_property<maui::core::thickness> descriptor{"padding"};
        return descriptor;
    }

    void content_presenter::set_content(std::shared_ptr<element> value)
    {
        // ContentPresenter.OnContentChanged: detach the old, attach the new. C#'s ParentOverride
        // redirection (newView.ParentOverride = templated parent) collapses into the push model —
        // the templated control pushes its BindingContext into the content directly, and this
        // presenter's set_child_inherited_binding_context never propagates its own.
        if (content_ == value)
        {
            return;
        }
        if (content_ != nullptr)
        {
            detach_logical_child(*content_);
        }
        content_ = std::move(value);
        if (content_ != nullptr)
        {
            attach_logical_child(*content_);
        }
        // Tell the handler to re-host the (changed) presented content. C# ContentPresenter is a
        // Compatibility.Layout whose ContentViewHandler hosts IContentView.PresentedContent (the
        // packed developer content) as a native subview; the port routes that re-host through the
        // same "set_content" command its sibling content hosts use (content_view / content_page). A
        // no-op when no handler is attached yet (the connect-time "content" property map then hosts).
        if (const auto& element_handler = handler())
        {
            element_handler->invoke("set_content");
        }
    }

    maui::graphics::size content_presenter::measure(double width_constraint, double height_constraint)
    {
        const maui::core::thickness inset = padding();
        maui::graphics::size content_size{0, 0};
        if (auto* presented = content())
        {
            content_size = presented->measure(width_constraint - inset.horizontal_thickness(),
                                              height_constraint - inset.vertical_thickness());
        }
        const maui::graphics::size measured{content_size.width + inset.horizontal_thickness(),
                                            content_size.height + inset.vertical_thickness()};
        desired_size_ = measured;
        return measured;
    }

    maui::graphics::size content_presenter::arrange(const maui::graphics::rect& bounds)
    {
        frame_ = bounds;
        if (auto* view_handler = dynamic_cast<maui::core::i_view_handler*>(handler().get()))
        {
            view_handler->platform_arrange(bounds);
        }
        if (auto* presented = content())
        {
            // The presented content is hosted as a SUBVIEW of the presenter's native host (which
            // platform_arrange framed at `bounds`). A subview's frame is expressed in its superview's
            // coordinate space, whose origin is (0,0) — so the content is arranged HOST-RELATIVE: the
            // padding inset from the host's top-left, with `bounds.x/bounds.y` dropped. Carrying the
            // absolute origin here would double-offset the content (host frame origin + the same origin
            // again), pushing a deeply-nested presenter's content off-screen. See templated_view::arrange
            // / border::arrange — these single-content hosts share the convention.
            const maui::core::thickness inset = padding();
            presented->arrange({inset.left, inset.top, bounds.width - inset.horizontal_thickness(),
                                bounds.height - inset.vertical_thickness()});
        }
        return {bounds.width, bounds.height};
    }
} // namespace maui::controls

// Self-register the default handler for content_presenter (opt-in, PROFILE §6). C# ContentPresenter
// (a Compatibility.Layout implementing IContentView) renders its single PresentedContent the same way
// ContentView does, so the port hosts it through the SAME content_page_handler (the ContentViewHandler
// port) its i_content_view siblings use — this is what lets a ControlTemplate's presenter actually
// mount the packed developer content as a native subview.
MAUI_REGISTER_HANDLER(maui::controls::content_presenter, maui::core::content_page_handler)
