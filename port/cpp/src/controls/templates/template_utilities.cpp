// maui::controls::template_utilities — the ControlTemplate application machinery
// (template_utilities.hpp). Ported from TemplateUtilities.cs (OnControlTemplateChanged's
// presenter-unbinding BFS + the create/attach/hook sequence, and OnContentChanged).
#include "maui/controls/templates/template_utilities.hpp"

#include <deque>
#include <memory>
#include <stdexcept>

#include "maui/controls/element.hpp"
#include "maui/controls/templates/content_presenter.hpp"
#include "maui/controls/templates/control_template.hpp"
#include "maui/controls/templates/i_control_templated.hpp"
#include "maui/core/i_view.hpp"

namespace maui::controls
{
    void template_utilities::remove_all_internal_children(element& self)
    {
        auto& templated = dynamic_cast<i_control_templated&>(self);
        while (!templated.internal_children().empty())
        {
            templated.remove_at(static_cast<int>(templated.internal_children().size()) - 1);
        }
    }

    void template_utilities::refresh_owned_presenters(element& owner)
    {
        // Visit every content_presenter in the logical subtree; one belonging to a NESTED template
        // scope resolves to the nested control and is filtered by the equality check.
        auto& templated = dynamic_cast<i_control_templated&>(owner);
        std::deque<element*> queue{&owner};
        while (!queue.empty())
        {
            element* current = queue.front();
            queue.pop_front();
            current->for_each_logical_child([&queue](element& child) { queue.push_back(&child); });
            if (auto* presenter = dynamic_cast<content_presenter*>(current);
                presenter != nullptr && presenter->find_templated_parent() == &owner)
            {
                presenter->set_content(templated.templated_content());
            }
        }
    }

    void template_utilities::on_control_template_changed(element& self, control_template* old_value)
    {
        auto& templated = dynamic_cast<i_control_templated&>(self);

        // 1. "First make sure any old ContentPresenters are no longer bound up" — BFS through the
        // logical children, clearing presenters and NOT descending into a child templated scope
        // (a child with its own ControlTemplate keeps its template intact).
        if (old_value != nullptr)
        {
            std::deque<element*> queue{&self};
            while (!queue.empty())
            {
                element* current = queue.front();
                queue.pop_front();
                current->for_each_logical_child([&queue](element& child) {
                    if (auto* presenter = dynamic_cast<content_presenter*>(&child))
                    {
                        presenter->clear();
                        return;
                    }
                    const auto* child_templated = dynamic_cast<const i_control_templated*>(&child);
                    if (child_templated == nullptr || child_templated->control_template() == nullptr)
                    {
                        queue.push_back(&child);
                    }
                });
            }
        }

        // 2. "Now remove all remnants of any other children just to be sure."
        remove_all_internal_children(self);

        // 3. Create + attach the new content, then the C# hook order: AddLogicalChild →
        // OnControlTemplateChanged → TemplateRoot → OnApplyTemplate. (The attach re-resolves the new
        // subtree's template bindings — including each presenter's TemplatedParent.Content pull.)
        control_template* new_value = templated.control_template().get();
        if (new_value == nullptr)
        {
            return; // C#: "do nothing for now"
        }
        std::shared_ptr<maui::core::bindable_object> created = new_value->create_content();
        auto content = std::dynamic_pointer_cast<element>(created);
        if (content == nullptr || dynamic_cast<maui::core::i_view*>(content.get()) == nullptr)
        {
            // C# NotSupportedException.
            throw std::runtime_error("control_template must return a type derived from View");
        }
        templated.add_logical_child(content);
        templated.on_control_template_changed(old_value, new_value);
        templated.set_template_root(content.get());
        templated.on_apply_template();
    }

    void template_utilities::on_content_changed(element& self, const std::shared_ptr<element>& new_content)
    {
        auto& templated = dynamic_cast<i_control_templated&>(self);
        if (templated.control_template() == nullptr)
        {
            // Untemplated: the content IS the logical child — drop everything else, then add it.
            // (The C# SwipeView carve-out — preserving SwipeItems — waits for SwipeView itself.)
            remove_all_internal_children(self);
            if (new_content != nullptr)
            {
                templated.add_logical_child(new_content);
            }
            return;
        }
        // Templated: the content inherits this control's BindingContext DIRECTLY (C#
        // SetInheritedBindingContext(newElement, bindable.BindingContext)) — it is presented inside the
        // template scope but belongs to the outer data context...
        if (new_content != nullptr)
        {
            new_content->set_inherited_binding_context(self.raw_binding_context());
        }
        // ...and every presenter bound to this control re-presents it (the C# presenter binding push).
        refresh_owned_presenters(self);
    }
} // namespace maui::controls
