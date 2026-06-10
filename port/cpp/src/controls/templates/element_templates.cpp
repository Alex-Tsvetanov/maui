// The element-side templated-parent members (the W1-09 block in element.hpp): the synchronous
// FindTemplatedParent walk, the template_binding storage/re-application, and the out-of-line
// destructor (the header only forward-declares template_binding). Ported from
// TemplateUtilities.FindTemplatedParentAsync and BindableObject.SetBinding's TemplateBinding path.
#include "maui/controls/element.hpp"

#include <utility>

#include "maui/controls/templates/content_presenter.hpp"
#include "maui/controls/templates/i_control_templated.hpp"
#include "maui/controls/templates/template_binding.hpp"

namespace maui::controls
{
    element::element() = default;  // out-of-line: template_bindings_ needs the complete type here
    element::~element() = default; // (both for the ctor's unwind path and the dtor itself)

    element* element::find_templated_parent() const
    {
        // TemplateUtilities.FindTemplatedParentAsync, synchronous: walk up from the parent; the first
        // templated ancestor wins, except that each content_presenter passed on the way up skips one
        // (a presenter's content belongs to the OUTER template scope). The C# Application guard does
        // not arise — the application is not part of the port's logical-parent chain.
        int skip_count = 0;
        for (element* current = logical_parent_; current != nullptr; current = current->logical_parent_)
        {
            if (const auto* templated = dynamic_cast<const i_control_templated*>(current);
                templated != nullptr && templated->control_template() != nullptr)
            {
                if (skip_count == 0)
                {
                    return current;
                }
                --skip_count;
            }
            if (dynamic_cast<const content_presenter*>(current) != nullptr)
            {
                ++skip_count;
            }
        }
        return nullptr;
    }

    void element::set_template_binding(template_binding binding)
    {
        // One binding per target property (C# SetBinding replaces); then apply against the current
        // chain immediately (C# binding.Apply on SetBinding).
        for (auto& existing : template_bindings_)
        {
            if (existing.target_property_name() == binding.target_property_name())
            {
                existing = std::move(binding);
                existing.apply(*this, find_templated_parent());
                return;
            }
        }
        template_bindings_.push_back(std::move(binding));
        template_bindings_.back().apply(*this, find_templated_parent());
    }

    void element::clear_template_bindings()
    {
        template_bindings_.clear(); // drops the subscriptions; bound values stay (C# RemoveBinding keeps)
    }

    void element::reapply_template_bindings()
    {
        if (template_bindings_.empty())
        {
            return;
        }
        element* templated_parent = find_templated_parent();
        for (auto& binding : template_bindings_)
        {
            binding.apply(*this, templated_parent);
        }
    }
} // namespace maui::controls
