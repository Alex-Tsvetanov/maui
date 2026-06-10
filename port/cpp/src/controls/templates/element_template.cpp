// maui::controls::element_template — CreateContent + the loader plumbing (element_template.hpp).
// Ported from ElementTemplate.cs.
#include "maui/controls/templates/element_template.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template_selector.hpp"
#include "maui/core/bindable_object.hpp"

namespace maui::controls
{
    element_template::element_template(loader load_template, bool can_recycle) : can_recycle_(can_recycle)
    {
        if (!load_template)
        {
            // C# ArgumentNullException(nameof(loadTemplate)).
            throw std::invalid_argument("element_template: loadTemplate must not be null");
        }
        load_template_ = std::move(load_template);
    }

    std::shared_ptr<maui::core::bindable_object> element_template::create_content() const
    {
        if (!load_template_)
        {
            // C# returns a Label instead of throwing so a mid-HotReload template never crashes the
            // CreateContent callers; mirrored verbatim.
            return std::make_shared<label>();
        }
        if (dynamic_cast<const data_template_selector*>(this) != nullptr)
        {
            // C# InvalidOperationException.
            throw std::runtime_error("Cannot call create_content directly on a data_template_selector");
        }
        std::shared_ptr<maui::core::bindable_object> item = load_template_();
        if (item != nullptr)
        {
            setup_content(*item);
            if (auto* element_item = dynamic_cast<element*>(item.get()))
            {
                element_item->set_is_template_root(true);
            }
        }
        return item;
    }
} // namespace maui::controls
